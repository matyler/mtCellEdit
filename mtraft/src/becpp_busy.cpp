/*
	Copyright (C) 2015 Mark Tyler

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program in the file COPYING.
*/

#include "becpp.h"



busyState :: busyState ()
	:
	status		( BUSY_IDLE )
{
}

busyState :: ~busyState ()
{
}

void busyState :: setWorking ()
{
	status = BUSY_WORKING;
}

void busyState :: setStopped ()
{
	status = BUSY_STOPPED;
}

void busyState :: setIdle ()
{
	status = BUSY_IDLE;
}

int busyState :: getStatus () const
{
	return status;
}

