/**
* @headerfile Message.h
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2010>
*
* @section LICENSE
* This file is part of OpenPedSim.
*
* OpenPedSim is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* OpenPedSim is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
*
* @section DESCRIPTION
*
* @brief contains messages that are exchanged between the different modules
*
*
*  Created on: 24.08.2009
*
*/

#ifndef MESSAGE_H_
#define MESSAGE_H_

class Message {

public :
    enum message {STACK_EMPTY=1,
                  STACK_REACHS_BEGINNING,
                  STACK_REACHS_END,
                  RESET,
                  VISUALISATION_TERMINATED

                 };
};

#endif /* MESSAGE_H_ */
