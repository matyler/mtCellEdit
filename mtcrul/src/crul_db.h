/*
	Copyright (C) 2020 Mark Tyler

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

#ifndef CRUL_DB_H
#define CRUL_DB_H

#include "crul.h"



#define DB_SCHEMA_VERSION	18

#define DB_TABLE_CACHE_PTS_IDX	"Cache_Points_Index"
#define DB_TABLE_RULER_LIST	"Ruler_List"
#define DB_TABLE_CAMERA_LIST	"Camera_List"

#define DB_FIELD_ID		"ID"
#define DB_FIELD_NAME		"Name"
#define DB_FIELD_TOTAL		"Total"

#define DB_FIELD_X1		"X1"
#define DB_FIELD_Y1		"Y1"
#define DB_FIELD_Z1		"Z1"
#define DB_FIELD_X2		"X2"
#define DB_FIELD_Y2		"Y2"
#define DB_FIELD_Z2		"Z2"
#define DB_FIELD_RGB		"RGB"
#define DB_FIELD_LABEL		"Label"
#define DB_FIELD_VISIBLE	"Visible"
#define DB_FIELD_PLANE		"Plane"

#define DB_FIELD_X		"X"
#define DB_FIELD_Y		"Y"
#define DB_FIELD_Z		"Z"
#define DB_FIELD_ROT_X		"Rot_X"
#define DB_FIELD_ROT_Z		"Rot_Z"
#define DB_FIELD_PROTECTED	"Protected"

#define DB_NAME_LOW		"Low"
#define DB_NAME_MEDIUM		"Medium"
#define DB_NAME_HIGH		"High"
#define DB_NAME_MODEL		"Model"



#endif		// CRUL_DB_H

