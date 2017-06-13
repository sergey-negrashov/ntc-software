/*
 *  This file is part of libmtc-data.
 *
 *  libmtc-data is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libmtc-data is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libmtc-data.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright Sergey Negrashov 2014.
*/

#include "../lib/dataoperations.hpp"
#include "../lib/util.hpp"
#include <stdexcept>
#include <math.h>
//#include <cmath>
#include <numeric>

using namespace std;

namespace mtc
{
namespace data
{

static uint16_t averageSamplesMakeIndex(uint8_t scrodId, uint8_t channel)
{
    int16_t out = 0;
    out |= scrodId << 16;
    out |= channel;
    return out;
}

/* Unused for now.
static std::pair<uint8_t, uint8_t> averageSamplesDecodeIndex(uint16_t index)
{
    uint8_t scrod = (index >> 16) & 0xFF;
    uint8_t channel = index & 0xFF;
    return std::make_pair(scrod, channel);
}
*/


typedef std::map < int16_t, std::pair<Window, int> > AverageCounterType;

Memory averageSamples(std::vector<DataStore*> &datas)
{
    AverageCounterType averageCounter;

    uint32_t event;
    uint8_t scrod;
    uint8_t channel;
    uint16_t window;
    Memory out;
    for(size_t file = 0; file< datas.size(); file++)
    {
        std::vector < uint32_t> events = datas[file]->events();
        BOOST_FOREACH(event, events)
        {
            std::vector < uint8_t> scrods = datas[file]->scrods(event);
            BOOST_FOREACH(scrod, scrods)
            {
                std::vector <uint8_t> channels = datas[file]->channels(event, scrod);
                BOOST_FOREACH(channel, channels)
                {
                    std::vector < uint16_t> windows = datas[file]->windows(event, scrod, channel);
                    std::sort(windows.begin(), windows.end());
                    for(size_t i = 0; i < windows.size(); i++)
                    {
                        window = windows[i];
                        Window w = datas[file]->window(event, scrod, channel, window);
                        uint16_t index = averageSamplesMakeIndex(w.scrodId, w.chanId);
                        if(averageCounter.find(index) == averageCounter.end())
                            averageCounter[index] = std::make_pair(w, 1);
                        else
                        {
                            averageCounter[index].first.add(w);
                            averageCounter[index].second++;
                        }
                    }
                }
            }
        }
    }
    BOOST_FOREACH(const AverageCounterType::value_type& windowDescriptor, averageCounter)
    {
        Window w = windowDescriptor.second.first;
        w.divideScalar(windowDescriptor.second.second);
        w.eventNum = 0xFFFFFFFF;
        out.addWindow(w);
    }
    return out;
}

uint64_t findNumberOfPulses(DataStore &data, uint32_t this_event) throw(std::runtime_error)
{
    uint64_t num_of_pulses = 0;
    std::vector < uint32_t> events = data.events();
    uint32_t event;
    uint8_t scrod;
    uint8_t channel;

    BOOST_FOREACH(event, events)
    {

        if(event == this_event)
        {
            std::vector < uint8_t> scrods = data.scrods(event);
            BOOST_FOREACH(scrod, scrods)
            {
                std::vector <uint8_t> channels = data.channels(event, scrod);
                BOOST_FOREACH(channel, channels)
                {

                    Curve cur = generateCurveNoTiming(data,event,scrod,channel);
                    //cout << "Finished finding the curve - voltages.size: " << cur.voltages.size() << endl;

                    std::vector<Pulse> pulses = findPulsesFromChannelNoTiming(cur,5,20);
                    cout << "We finished finding pulses." << endl;
                    //cout << "pulses.length: " << pulses.size() << endl;
                    num_of_pulses += pulses.size();

                }
            }
        }
    }


    return num_of_pulses;
}


std::vector<Pulse> findPulsesFromChannelNoTiming(Curve &c, uint minWidth,int minHeight, float threshold, float step) throw(std::runtime_error)
{
    std::vector<Pulse> ret;
    std::pair<int, int> roi= std::make_pair(0,0);
    std::vector<std::pair<int,int> > ROIs;
    size_t roi_half_width = 7;
    bool first = true;
    for(size_t i = 0; i < c.voltages.size(); i++)   //cycle through voltage array to look for a pulse region of interest
    {
        if(c.voltages[i] > minHeight)               //Does a voltage pass the height cut?
        {
            if(first)                               //Is it's the first, keep going. else statement will check width when pulse dips below minHeight again
            {
                if(i < roi_half_width)                           //if you're at the beginning of the array, keep going
                    continue;
                roi.first = i - roi_half_width;                  //if not, mark this spot in voltage array, minus 5, as beginning of ROI
                first = false;                      //Note it's not the first high value anymore
            }
        }
        else                                        //if this voltage doesn't meet min heigh requirement anymore...
        {
            if(!first)
            {
                if((i - roi.first - roi_half_width) > minWidth)  //With minWidth usually 5, should it be a -5 (was +5)? if it's not beginning of array, this Voltage is far enough away (wide) in V array...
                {
                    if(i + roi_half_width >= c.voltages.size()) //if pulse isn't fully contained in array (part is outside), move on
                        continue;

                    roi.second = i + roi_half_width;             //if pulse is contained, add 5 to ending "i" where pulse dips below minHeight, you've got your ROI
                    ROIs.push_back(roi);
                    first = true;
                }
                else
                    first = true;
            }
        }
    }

    std::vector<float> temp;
    for(size_t roi = 0; roi < ROIs.size(); roi++)       //Now you've found your regions of interest, candidate pulses, lets analyze
    {
        temp.clear();
        for(int j = ROIs[roi].first; j < ROIs[roi].second; j++)
        {
            temp.push_back(c.voltages[j]);
        }
        float max = *std::max_element(temp.begin(), temp.end());
        float target = max*threshold;       //default threshold = 0.4, but can be put in the function as an argument
        float target_fttime = 100;          //set pretty low, since we get some rather low pulses
        float start = 0;
        float startft = 0;
        float end = 0;
        uint start_counting_charge = -1;
        uint charge_count_steps = 0;
        float q_sums = 0;      //counting charge to do trapezoidal rule integral

        for(uint j = 0; j < temp.size() -1; j++)
        {
            if(start == 0)
            {
                if(temp[j] < target && temp[j+1] > target)  //if this V is less than threshold starting point, but next is higher, start here
                {
                    start = j + (target - temp[j])/(temp[j+1] - temp[j]);
                    //j+=5; //why???
                    if(start_counting_charge < 0)
                    {
                        start_counting_charge = j;
                    }
                }
                if(temp[j] < target_fttime && temp[j+1] > target_fttime) //if this V is less than fixed_threshold starting point, but next is higher, start here
                {
                    startft = j + (target_fttime - temp[j])/(temp[j+1] - temp[j]);
                    start_counting_charge = j;
                    //j+=5;
                }
                q_sums += temp[j];
            }
            if(startft == 0)
            {
                if(temp[j] < target_fttime && temp[j+1] > target_fttime) //if this V is less than fixed_threshold starting point, but next is higher, start here
                {
                    startft = j + (target_fttime - temp[j])/(temp[j+1] - temp[j]);
                    //start_counting_charge = 1;
                    //j+=5;
                }
                q_sums += temp[j];

            }
            if(start != 0 && startft != 0)
            {
                q_sums += temp[j];
                if(temp[j] > target && temp[j+1] < target)
                {
                    end = j + (target - temp[j])/(temp[j+1] - temp[j]);
                    charge_count_steps = j - start_counting_charge;
                    break;
                }
            }
            /*
            if(start_counting_charge >= 0)
            {
                q_sums += temp[j];  //adding Voltages/counts along pulse
            }*/
        }
        if(end > 0)         //this if statement isn't going off (caused by two if statements up), causing pulses to be lost
        {
            Pulse p;
            p.event = c.event;
            p.chan = c.channel;
            p.scrod = c.scrod;
            p.maxValue = max;
            //if(ROIs[roi].first < 0) //experimental step. If else block wasn't originally there.
            p.crossingTime = c.times[ROIs[roi].first] + start*step; //calculate crossingTime ->
            p.width = end - start;
            p.window = c.windows[(ROIs[roi].first + start)/512];
            p.windowPosition = fmod((ROIs[roi].first + start), 512);
            p.fixedThreshTime = c.times[ROIs[roi].first] + startft*step;
            p.charge_simp = (0.5*p.maxValue)*(p.width); //simple area, as a trianlge
            p.charge = q_sums*(p.width/charge_count_steps); //area taken via trapezoidal rule
            //cout << "Q_sum: " << q_sums << "   Width: " << p.width << "   steps: " << charge_count_steps << "   charge: " << p.charge << "   simp_q " << p.charge_simp << endl;
            ret.push_back(p);
        }
        /*else if(end == -2000)   //A backup case if end never gets set. Needs work
        {
            end = 0;
            Pulse p;
            p.event = c.event;
            p.chan = c.channel;
            p.scrod = c.scrod;
            p.maxValue = max;
            //if(ROIs[roi].first < 0) //experimental step. If else block wasn't originally there.
            p.crossingTime = c.times[ROIs[roi].first] + start*step; //calculate crossingTime ->
            p.width = end - start;
            p.window = c.windows[(ROIs[roi].first + start)/512];
            p.windowPosition = fmod((ROIs[roi].first + start), 512);
            p.fixedThreshTime = c.times[ROIs[roi].first] + startft*step;
            p.charge_simp = (0.5*p.maxValue)*(p.width); //simple area, as a trianlge
            p.charge = q_sums*(p.width/temp.size()); //area taken via trapezoidal rule
            cout << "Q_sum: " << q_sums << "   Width: " << p.width << "   steps: " << charge_count_steps << "   charge: " << p.charge << "   simp_q " << p.charge_simp << endl;
            ret.push_back(p);

        }*/
    }
    //cout << "At end of find pulses before return ." << endl;
    return ret;
}

Curve generateCurveNoTiming(DataStore &data, int32_t event, uint8_t scrod, int8_t channel, PmtMap map, float step) throw(std::runtime_error)
{
    Curve out;
	uint16_t window;
    out.event = event;
    out.scrod = scrod;
    out.channel = channel;
    GlobalChannel gc;
    gc.scrod = scrod;
    gc.channel = channel;
    out.pmt = map.getPmtChannel(gc);
    std::vector < uint16_t> windows = data.windows(event, scrod, channel);
    std::sort(windows.begin(), windows.end());

    BOOST_FOREACH(window, windows)
    {
		Window w = data.window(event,scrod,channel,window);
        if(w.trgBit > 0)
        {
			out.trgBit = w.trgBit;
        }
    }

    uint16_t refWindow = 0;
    if(windows.size() != 0)
    {
        refWindow = data.window(event, scrod, channel, windows[0]).referenceWindow;
    }

    std::vector < uint16_t> orderedWindows;
    BOOST_FOREACH(uint16_t windowId, windows)
    {
        if(windowId >= refWindow)
        {
            orderedWindows.push_back(windowId);
        }
    }
    BOOST_FOREACH(uint16_t windowId, windows)
    {
        if(windowId < refWindow)
        {
            orderedWindows.push_back(windowId);
        }
    }
    windows = orderedWindows;
    BOOST_FOREACH(uint16_t windowId, windows)
    {
        Window w = data.window(event, scrod, channel, windowId);
        float t;
        if(windowId >= refWindow)
        {
            t = (windowId- refWindow) *w.values.size();
            t *= step;
        }
        else
        {
            t = (MAX_WINDOW_ID_FOR_IRS2 - refWindow + windowId) *w.values.size();
            t *= step;
        }
        out.windows.push_back(windowId);
        out.refWindow = refWindow;
        for(size_t i = 0; i < w.values.size(); i++)
        {
            out.voltages.push_back(w.values[i]);
            out.times.push_back(t);
            t+= step;
        }

    }
    return out;
}

/*
std::vector<Curve> generateCurvesNoTiming(DataStore &data, int32_t event, uint8_t scrod, int8_t channel, PmtMap map, float step) throw(std::runtime_error)
{
    std::vector<Pulse> curveVec;
    Curve out;
    uint16_t window;
    out.event = event;
    out.scrod = scrod;
    out.channel = channel;
    GlobalChannel gc;
    gc.scrod = scrod;
    gc.channel = channel;
    out.pmt = map.getPmtChannel(gc);
    std::vector < uint16_t> windows = data.windows(event, scrod, channel);
    std::sort(windows.begin(), windows.end());

    BOOST_FOREACH(window, windows)
    {
        Window w = data.window(event,scrod,channel,window);
        if(w.trgBit > 0)
        {
            out.trgBit = w.trgBit;
        }
    }

    uint16_t refWindow = 0;
    if(windows.size() != 0)
    {
        refWindow = data.window(event, scrod, channel, windows[0]).referenceWindow;
    }   //ref window is first window in array for any pulses found here

    std::vector < uint16_t> orderedWindows;
    BOOST_FOREACH(uint16_t windowId, windows)   //begin ordering windows in case of wrap around at 512
    {
        if(windowId >= refWindow)
        {
            orderedWindows.push_back(windowId);
        }
    }
    BOOST_FOREACH(uint16_t windowId, windows)
    {
        if(windowId < refWindow)
        {
            orderedWindows.push_back(windowId);
        }
    }
    windows = orderedWindows;
    BOOST_FOREACH(uint16_t windowId, windows)   //windows now order, going through the find curves
    {
        Window w = data.window(event, scrod, channel, windowId);
        float t;
        if(windowId >= refWindow)
        {
            t = (windowId- refWindow) *w.values.size();
            t *= step;
        }
        else
        {
            t = (MAX_WINDOW_ID_FOR_IRS2 - refWindow + windowId) *w.values.size();
            t *= step;
        }
        out.windows.push_back(windowId);
        out.refWindow = refWindow;
        for(size_t i = 0; i < w.values.size(); i++)
        {
            out.voltages.push_back(w.values[i]);
            out.times.push_back(t);
            t+= step;
        }

    }
    curveVec.push_back(out);
    //return out;
    return curveVec;
} */


Memory pruneChannels(DataStore &data, ChannelIgnoreMask &mask)
{
    uint32_t event;
    uint8_t scrod;
    uint8_t channel;
    uint16_t window;

    Memory out;
    std::vector < uint32_t> events = data.events();

    BOOST_FOREACH(event, events)
    {
        std::vector < uint8_t> scrods = data.scrods(event);
        BOOST_FOREACH(scrod, scrods)
        {
            try
            {
                DataPacket headerPacket = data.getEventHeader(event, scrod);
                out.addPacket(headerPacket);
            }
            catch(runtime_error &e){}
            std::vector <uint8_t> channels = data.channels(event, scrod);
            BOOST_FOREACH(channel, channels)
            {
                std::vector < uint16_t> windows = data.windows(event, scrod, channel);
                std::sort(windows.begin(), windows.end());
                for(size_t i = 0; i < windows.size(); i++)
                {
                    window = windows[i];
                    ChannelIgnoreMask::Mask m = std::make_pair(scrod, (uint)channel << 8 | (uint)window);
                    if(std::find(mask.maskList.begin(), mask.maskList.end(), m) == mask.maskList.end())
                    {
                        Window w = data.window(event, scrod, channel, window);
                        out.addWindow(w);
                    }
                }
            }
        }
    }
    return out;
}

Memory subtractPedistal(DataStore &data, DataStore &ped)
{
    uint32_t event;
    uint32_t pedEvent;
    uint8_t scrod;
    uint8_t channel;
    uint16_t window;
    float counterthing = 0;

    Memory out;

    pedEvent = ped.events()[0]; //assumes pedestal is stored as one large event aka event 0 in the pedestal file
    //pedEvents = ped.events();

    std::vector < uint32_t> events = data.events();
    //cout << "Preparing to subtract." << endl;

    BOOST_FOREACH(event, events)
    {
        std::vector < uint8_t> scrods = data.scrods(event);
        BOOST_FOREACH(scrod, scrods)
        {
            try
            {
                DataPacket headerPacket = data.getEventHeader(event, scrod);
                out.addPacket(headerPacket);
            }
            catch(runtime_error &e){}
            std::vector <uint8_t> channels = data.channels(event, scrod);
            BOOST_FOREACH(channel, channels)
            {
                counterthing++;
                //cout << "Subtracting windows!" << counterthing << endl;
                std::vector < uint16_t> windows = data.windows(event, scrod, channel);
                for(size_t i = 0; i < windows.size(); i++)
                {
                    window = windows[i];
                    Window w = data.window(event, scrod, channel, window);
                    try
                    {
                        Window p = ped.window(pedEvent, scrod, channel, window);//Previously always expected one giant event -pedEvent -> event[0] in pedestal file events
                        w.subtract(p);
                        out.addWindow(w);
                    }
                    catch(runtime_error &e)
                    {
                        continue;
                    }
                }
            }
        }
    }
    cout << "Pedestals subtracted!" << endl;
    return out;
}

Memory subtractPedistal(Memory &data, DataStore &ped)
{
    uint32_t event;
    uint32_t pedEvent;
    uint8_t scrod;
    uint8_t channel;
    uint16_t window;
    float counterthing = 0;

    Memory out;

    pedEvent = ped.events()[0]; //assumes pedestal is stored as one large event aka event 0 in the pedestal file
    //pedEvents = ped.events();

    std::vector < uint32_t> events = data.events();
    //cout << "Preparing to subtract." << endl;

    BOOST_FOREACH(event, events)
    {
        std::vector < uint8_t> scrods = data.scrods(event);
        BOOST_FOREACH(scrod, scrods)
        {
            try
            {
                DataPacket headerPacket = data.getEventHeader(event, scrod);
                out.addPacket(headerPacket);
            }
            catch(runtime_error &e){}
            std::vector <uint8_t> channels = data.channels(event, scrod);
            BOOST_FOREACH(channel, channels)
            {
                counterthing++;
                //cout << "Subtracting windows!" << counterthing << endl;
                std::vector < uint16_t> windows = data.windows(event, scrod, channel);
                for(size_t i = 0; i < windows.size(); i++)
                {
                    window = windows[i];
                    Window w = data.window(event, scrod, channel, window);
                    try
                    {
                        Window p = ped.window(pedEvent, scrod, channel, window);//Previously always expected one giant event -pedEvent -> event[0] in pedestal file events
                        w.subtract(p);
                        out.addWindow(w);
                    }
                    catch(runtime_error &e)
                    {
                        continue;
                    }
                }
            }
        }
    }
    cout << "Pedestals subtracted!" << endl;
    return out;
}

float FirstTrigBit(uint16_t input) //used in skimEvent, trimBadEvents
{
   if (input == 1)  return 419-3;
   if (input == 3)  return 419-2;
   if (input == 7)  return 419-1;
   if (input == 15) return 419;
   if (input == 14) return 419+1;
   if (input == 12) return 419+2;
   if (input == 8)  return 419+3;
   return -1;
}

bool skimEventCopy(Index &data, uint32_t event) //Used to determine whether a particular event is muonlike or not and should be skimmed from text conversions (.glenn, .dst)
{

	int32_t promptDelayedCutoff = 400;
	int32_t firstWindowDelayed  = 419;

	uint16_t trgBitMap[12*128] = {0};
	uint16_t trgBitTot[12*128] = {0};
	uint16_t trgBitMapP[12*128] = {0};
	uint16_t trgBitTotD[12*128] = {0};

	uint8_t scrod;
	uint8_t channel;
	uint16_t window;

	std::vector < uint8_t> scrods = data.scrods(event);
	BOOST_FOREACH(scrod, scrods)
	{
		std::vector <uint8_t> channels = data.channels(event, scrod);
		BOOST_FOREACH(channel, channels)
		{
			uint32_t flatChannel = uint32_t(scrod)*128 + uint32_t(channel);
			std::vector<uint16_t> windows =data.windows(event,scrod,channel);
			BOOST_FOREACH(window, windows)
			{
				Window w = data.window(event,scrod,channel,window);

				int32_t relWin = (int32_t(w.windowId) - int32_t(w.referenceWindow) + 512) % 512;

				if (w.trgBit && relWin > promptDelayedCutoff)  //419-422
				{
					uint32_t bitShift = relWin-firstWindowDelayed;
					trgBitMap[flatChannel] += (0x1 << bitShift);
					trgBitTotD[flatChannel] = 1;
				}
				if (w.trgBit && relWin < promptDelayedCutoff) 
				{
					uint32_t bitShift = relWin-firstWindowDelayed;
					trgBitMapP[flatChannel] += (0x1 << bitShift);
					trgBitTot[flatChannel] = 1;
				}

			}
		}
	}

	int32_t totalP = 0;
	int32_t totalD = 0;
	std::vector<float> triggerStartsD(12*128);
	double rms = -1;
	double var = 0;
	double sum = 0;
	double sumSq = 0;
	double offset = 420;

   // Run through and check stats for this event
	for (int s = 0; s < 12*128; ++s) 
	{
		if (trgBitTot[s] == 1)
        {
			totalP++;
        }
		if (trgBitTotD[s] == 1) 
		{
			totalD++;
		}

		float firstHitD = FirstTrigBit(trgBitMap[s]);
		// Handle weird cases where we have a hit in the total but it's not following an expected pattern in window space
		if (firstHitD < 0 && trgBitTotD[s] == 1) 
		{
			totalD--;
		}

		if (firstHitD >= 0) 
		{
			triggerStartsD.push_back(firstHitD - offset);
			sum += firstHitD - offset;
		}
	}

	sumSq = std::inner_product(triggerStartsD.begin(),triggerStartsD.end(),triggerStartsD.begin(),0);
	if (totalD > 1 && totalP > 0) 
	{
		var = sumSq/double(totalD) - pow(sum/double(totalD),2);
		rms = sqrt(var);
        //Uncomment this out if you want to do some diagnostics
		//std::cout << event << "\t" << totalP << "\t" << totalD << "\t" << rms << std::endl;
	}

   // Skim these away (muons and junk events)
	if (totalP > 650 || rms > 1) 
	{
		return true;
	} 
	// Skim these away (no prompt event)
	if (totalP < 10) 
	{
		return true;
	}
	
	// Keep whatever's left
	return false;

}

Memory trimBadEvents(DataStore &data)
{
	int32_t promptDelayedCutoff = 400;
	int32_t firstWindowDelayed  = 419;

    uint32_t event;
    uint8_t scrod;
    uint8_t channel;
    uint16_t window;
	uint16_t refWindow;
    uint32_t totalEvents = 0;
    uint32_t skippedEvents = 0;

	uint16_t flatChannel = 0;

	std::vector < uint16_t> windows;
    std::vector <uint8_t> channels;

    Memory out; //For returning/handling the trimmed data
    std::vector <uint32_t> events = data.events();

    BOOST_FOREACH(event, events)
    {
        uint16_t trgBitMap[12*128] = {0};
        uint16_t trgBitTot[12*128] = {0};
        uint16_t trgBitMapP[12*128] = {0};
        uint16_t trgBitTotD[12*128] = {0};
        std::vector < uint8_t> scrods = data.scrods(event);
        BOOST_FOREACH(scrod, scrods)
        {
            try
            {
                DataPacket headerPacket = data.getEventHeader(event, scrod);
                out.addPacket(headerPacket);
            }
            catch(runtime_error &e){}
            channels = data.channels(event, scrod);
            BOOST_FOREACH(channel, channels)
            {
                //Channel c = Channel::decodeChannel(channel);
                flatChannel = uint32_t(scrod)*128 + uint32_t(channel);
                windows = data.windows(event,scrod,channel);
				if(windows.size() != 0)
                {
		            for(size_t i = 0; i < windows.size(); i++)
		            {
		                window = windows[i];
		                Window w = data.window(event, scrod, channel, window);
							 refWindow = w.referenceWindow;
                        uint16_t relWin = (window - refWindow + 512) % 512;
						if(w.trgBit == 1 && relWin <= promptDelayedCutoff)
						{
							uint32_t bitShift = relWin-firstWindowDelayed;
							trgBitMapP[flatChannel] += (0x1 << bitShift);
							trgBitTot[flatChannel] = 1;
						}
						else if(w.trgBit == 1 && relWin > promptDelayedCutoff)
						{
							uint32_t bitShift = relWin-firstWindowDelayed;
							trgBitMap[flatChannel] += (0x1 << bitShift);
							trgBitTotD[flatChannel] = 1;
						}

		            }
				}
            }
        } 
        //end of scrods loop, still in event loop
        // now observing stats, deciding whether to keep event or not
        int32_t totalP = 0;
        int32_t totalD = 0;
        std::vector<float> triggerStartsD(12*128);	//marking the start of the delayed triggers, so we can see how many we got, where the event really started
        double rms = -1;
        double var = 0;
        double sum = 0;
        double sumSq = 0;
        double offset = 420;

        for (int s = 0; s < 12*128; ++s) //Going channel by channel through the event details we saved
        {
            if (trgBitTot[s] == 1)
            {
                totalP++;
            }
            if (trgBitTotD[s] == 1)
            {
                totalD++;
            }

            float firstHitD = FirstTrigBit(trgBitMap[s]);
            // Handle weird cases where we have a hit in the total but it's not following an expected pattern in window space
            if (firstHitD < 0 && trgBitTotD[s] == 1)
            {
                totalD--;
            }

            if (firstHitD >= 0)
            {
                triggerStartsD.push_back(firstHitD - offset);
                sum += firstHitD - offset;
            }
        }

        sumSq = std::inner_product(triggerStartsD.begin(),triggerStartsD.end(),triggerStartsD.begin(),0);
        if (totalD > 1 && totalP > 0)
        {
            var = sumSq/double(totalD) - pow(sum/double(totalD),2);
            rms = sqrt(var);
            //uncomment this if you want to do some diagnostics
            std::cout << event << "\t" << totalP << "\t" << totalD << "\t" << rms << std::endl;
        }

        // Skim these away (muons and junk events)
        if (totalP > 650 || rms > 1) //For event where most channels are hit (prompt hits on more than 650 channels, cut them
        {
            skippedEvents+=1;
        }
        else if (totalP < 10) 	// Skim these away (no prompt event).
        {
            skippedEvents+=1;
        }
        else		// Keep whatever's left -> run through again, add event to output to be passed to other functions
        {
            totalEvents+=1;
            BOOST_FOREACH(scrod, scrods)
            {
                try
                {
                    DataPacket headerPacket = data.getEventHeader(event, scrod);
                    out.addPacket(headerPacket);
                }
                catch(runtime_error &e){}
                BOOST_FOREACH(channel, channels)
                {
                    std::vector < uint16_t> windows = data.windows(event, scrod, channel);
                    for(size_t i = 0; i < windows.size(); i++)
                    {
                        window = windows[i];
                        Window w = data.window(event, scrod, channel, window);
                        out.addWindow(w);	//add the windows to out

                    }
                }
            }
        }
    }
    cout << "Data trimmed! Total events: " << events.size() << " \t events saved: " << totalEvents << "\t skipped events: " << skippedEvents << endl;
    return out;
}



//Final brackets for keeping these functions in mtc namespace and mtc data space

}
}
