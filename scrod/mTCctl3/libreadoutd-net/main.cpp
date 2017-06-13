#include "lib/scrodnet.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

using namespace mtc::net2;

int main()
{
    int i;
    try
    {
        ScrodNet s(10000, "192.168.0.102");
//        std::vector<CardInfo> infos = s.getCardInfo();
//        s.setReg(0,0,3, 1);
//        cout << hex << s.getReg(0,0,3) << endl;
//        s.clearVeto();
//        s.enableCal(true);
//        s.enableCal(false);
//        s.enableVeto(true);
//        s.enableVeto(false);
//        cout << s.getCal() << endl;
//        cout << s.getCoarseDelay() << endl;
//        cout << s.getDelay() << endl;
//        cout << s.getTriggerCount() << endl;
//        cout << s.getTriggerDelay() << endl;
//        s.softTrigger();
//        s.setCoarseDelay(1);
//        s.setDelay(1);
//        s.setTriggerDelay(10);
//        s.setTriggerMask(0xFF00);
//        s.setTriggerMin(12);
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 4; j++)
{
			try
			{
			s.setReg(i,j,171,1);
			}
			catch(std::runtime_error &e){}
}
	 s.upgrade();
	while(true)
	{
		try
		{
		uint32_t* t = s.readPacket();
		delete t;
		}
		catch(std::runtime_error &e){}
	}
    }
    catch( std::runtime_error &e)
    {
        cout << e.what() << " " << i << endl;
    }
}
