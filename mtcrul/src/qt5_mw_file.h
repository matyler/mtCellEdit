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

#ifndef QT5_MW_FILE_H
#define QT5_MW_FILE_H

#include "qt5_mw.h"



class ThreadLoadModel;
class ThreadLoadPts;
class ThreadOpenDB;



class ThreadLoadModel : public QThread
{
	Q_OBJECT

public:
	explicit ThreadLoadModel ( Frontend & fe )
		:
		m_fe	( fe )
	{}

	void run ();

private:
	Frontend	& m_fe;
};



class ThreadLoadPts : public QThread
{
	Q_OBJECT

public:
	ThreadLoadPts (
		Frontend	& fe,
		int	const	type
		)
		:
		m_fe		( fe ),
		m_type		( type )
	{}

	void run ();

private:
	Frontend	& m_fe;
	int	const	m_type;
};



class ThreadOpenDB : public QThread
{
	Q_OBJECT

public:
	ThreadOpenDB (
		std::string	const	& filename,
		Frontend		& fe
		)
		:
		m_filename	( filename ),
		m_fe		( fe ),
		m_error		( 0 )
	{}

	void run ();

	inline int error () const { return m_error; }

private:
	std::string	const	& m_filename;
	Frontend		& m_fe;
	int			m_error;
};



#endif		// QT5_MW_FILE_H

