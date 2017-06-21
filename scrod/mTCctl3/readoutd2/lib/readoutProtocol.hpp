/*
 * This file is part of readoutd.
 *
 * readoutd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *
 * readoutd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with readoutd.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright Sergey Negrashov 2014.
*/

#ifndef READOUTPROTOCOL2_HPP
#define READOUTPROTOCOL2_HPP

#include <stdint.h>

/**
 * @brief The OPERRATION enum defines the dsp command type.
 */
enum  OPERRATION {GET_REG = 0,
                  SET_REG = 1,
                  GET_INFO = 2,
                  STORAGE_UPGRADE = 3,
                  TRG_MIN_GET = 4,
                  TRG_MIN_SET = 5,
                  SUBSCRIBE=6,
                  CAL_DELAY_GET = 7,
                  CAL_DELAY_SET = 8,
                  TRG_MASK_GET = 9,
                  TRG_MASK_SET = 10,
                  TRG_COUNT_GET = 11,
                  TRG_SOFT = 12,
                  TRG_VETO_EN_GET = 13,
                  TRG_VETO_EN_SET = 14,
                  TRG_VETO_CLEAR = 15,
                  TRG_VETO_GET = 16,
                  CAL_EN_GET = 17,
                  CAL_EN_SET = 18,
                  CAL_TRG_DELAY_GET = 19,
                  CAL_TRG_DELAY_SET = 20,
                  CAL_COARSE_DELAY_GET = 21,
                  CAL_COARSE_DELAY_SET = 22,
                  STORAGE_ROTATE = 23,
                  GET_TRG_EN = 24,
                  GET_MIN_A = 25,
                  GET_MAX_A = 26,
                  GET_MIN_B = 27,
                  GET_MAX_B = 28,
                  GET_MAX_DELAY_AB = 29,
                  GET_MIN_C = 30,
                  GET_MAX_C = 31,
                  SET_TRG_EN = 32,
                  SET_MIN_A = 33,
                  SET_MAX_A = 34,
                  SET_MIN_B = 35,
                  SET_MAX_B = 36,
                  SET_MAX_DELAY_AB = 37,
                  SET_MIN_C = 38,
                  SET_MAX_C = 39,
                  GET_PRESCALE_A = 40,
                  SET_PRESCALE_A = 41,
                  GET_PRESCALE_B = 42,
                  SET_PRESCALE_B = 43,
                  GET_PRESCALE_C = 44,
                  SET_PRESCALE_C = 45,
                  GET_PRESCALE_AB = 46,
                  SET_PRESCALE_AB = 47,
                  GET_TRG_LINK_STATUS = 48,
                  GET_LIVE_TIME = 49,
                  GET_TRG_A_RATE = 50,
                  GET_TRG_B_RATE = 51,
                  GET_TRG_C_RATE = 52,
                  GET_TRG_AB_RATE = 53,
                  GET_MIN_DELAY_AB = 54,
                  SET_MIN_DELAY_AB = 55,
                  ADVANCE_MOTOR = 56
                 };

/**
 * @brief PCI_ERROR internal pci error.
 */
static const uint16_t PCI_ERROR = 0;

/**
 * @brief TIMEOUT_ERROR responce to the command was not recieved.
 */
static const uint16_t TIMEOUT_ERROR = 1 << 8;
/**
 * @brief BAD_COMMAND_ERROR invalid command.
 */
static const uint16_t BAD_COMMAND_ERROR = 1<< 8;


/**
 * @brief The DspCommand struct fixed size command network structure.
 */
struct DspCommand
{
    /** Operation. */
    OPERRATION op;
    /** Register address. */
    uint16_t address;
    /** Register value in case of set. */
    uint16_t arg;
    /** Dspcpci card id. */
    int card;
    /** Channel for the dspcpci card. */
    int chan;
};

/**
 * @brief The DspResponce struct responce for a GET_REG or SET_REG command.
 */
struct DspResponce
{
    /** Operation status. */
    bool ok;
    /** Register address. */
    uint16_t address;
    /** Register value. */
    uint16_t arg;
    /** Card id. */
    int card;
    /** Channel number */
    int chan;
};

/**
 * @brief The DspInfoHeader struct responce to the GET_INFO command.
 */
struct DspInfoHeader
{
    /** Number of the dspcpci cards. */
    int cardNum;
};

/**
 * @brief The DspCardInfo struct responce body for the GET_INFO command.
 * There are cardNum structures following DspInfoHeader.
 */
struct DspCardInfo
{
    uint8_t id;
    uint8_t chan;
    uint32_t ioIn;
    uint32_t ioOut;
};

struct DspSubscription
{
    uint16_t scrod;
    uint32_t row0;
    uint32_t row1;
    uint32_t row2;
    uint32_t row3;
};

#endif // READOUTPROTOCOL2_HPP
