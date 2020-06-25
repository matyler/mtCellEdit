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

#ifndef QT5_MW_EDIT_H
#define QT5_MW_EDIT_H

#include "qt5_mw.h"



class ThreadImportModel;
class ThreadImportPTS;



class ThreadImportModel : public QThread
{
	Q_OBJECT

public:
	ThreadImportModel (
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



class ThreadImportPTS : public QThread
{
	Q_OBJECT

public:
	ThreadImportPTS (
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



#endif		// QT5_MW_EDIT_H

