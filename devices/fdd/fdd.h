/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2020 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef	__FDD_H__
#define	__FDD_H__

#include "../../platform/endian.h"

#pragma once

//*****************************************************************************
//	eUdi
//-----------------------------------------------------------------------------
class eUdi
{
public:
	eUdi(int cyls, int sides);
	~eUdi() { SAFE_DELETE_ARRAY(raw); }
	int Cyls() const	{ return cyls; }
	int Sides() const	{ return sides; }
	bool Changed() const { return changed; }
	void Changed(bool c) { changed = c; }

	enum { MAX_CYL = 86, MAX_SIDE = 2, MAX_SEC = 32 };
	struct eTrack
	{
		eTrack() : data_len(6400), data(NULL), id(NULL), sectors_amount(0) {}
		bool Marker(int pos) const;
		void Write(int pos, byte v, bool marker = false);
		void Update(); //on raw changed

		int		data_len;
		byte*	data;
		byte*	id;

		struct eSector
		{
			eSector() : id(NULL), data(NULL) {}
			enum eId { ID_CYL = 0, ID_SIDE, ID_SEC, ID_LEN, ID_CRC };
			int Cyl() const		{ return id[ID_CYL]; }
			int Side() const	{ return id[ID_SIDE]; }
			int Sec() const		{ return id[ID_SEC]; }
			int Len() const		{ return 128 << (id[ID_LEN] & 3); }
			word IdCrc() const	{ return WordBE(id + ID_CRC); }
			word DataCrc() const{ return WordBE(data + Len()); }
			word DataW(size_t offset) const
			{
				return Word(data + offset);
			}
			void DataW(size_t offset, word d)
			{
				data[offset] = d & 0xff;
				data[offset + 1] = d >> 8;
			}
			void UpdateCRC();
			
			byte*	id;
			byte*	data;
		};
		eSector* GetSector(int sec); // get logical sector by id[ID_SEC]
		bool	WriteSector(int sec, const byte* data); // write to logical sector
		eSector	sectors[MAX_SEC];
		int		sectors_amount;
	};
	eTrack& Track(int cyl, int side) { return tracks[cyl][side]; }

protected:
	int		cyls;
	int		sides;
	eTrack	tracks[MAX_CYL + 1][MAX_SIDE]; // 1 track beyond
	byte*	raw;
	bool	changed;
};

//*****************************************************************************
//	eFdd
//-----------------------------------------------------------------------------
class eFdd
{
public:
	eFdd();
	~eFdd() { SAFE_DELETE(disk); }
	qword Motor() const { return motor; }
	void Motor(qword v) { motor = v; }
	void Seek(int _cyl, int _side);
	int TSByte() const { return ts_byte; }
	int Cyl() const { return cyl; }
	void Cyl(int v) { cyl = v; }
	eUdi::eTrack& Track() { return disk->Track(cyl, side); }
	void Write(int pos, byte v, bool marker = false) { Track().Write(pos, v, marker); disk->Changed(true); }

	bool DiskPresent() const	{ return disk != NULL; }
	bool WriteProtect() const	{ return write_protect; }
	bool Open(const char* type, const void* data, size_t data_size);
	bool Store(const char* type, FILE* file) const;
	bool Bootable() const;
	bool DiskChanged() const;

	static word Crc(byte* src, int size);
	
protected:
	void WriteBlock(int& pos, byte v, int amount, bool marker = false)
	{
		for(int i = 0; i < amount; ++i)
		{
			Track().Write(pos++, v, marker);
		}
	}
	void Create(int max_cyl, int max_side);
	bool AddFile(const byte* hdr, const byte* data);
	bool ReadScl(const void* data, size_t data_size);
	bool ReadTrd(const void* data, size_t data_size);
	bool ReadFdi(const void* data, size_t data_size);
	bool ReadUdi(const void* data, size_t data_size);
	bool ReadTd0(const void* data, size_t data_size);

	bool WriteScl(FILE* file) const;
	bool WriteTrd(FILE* file) const;
	bool WriteFdi(FILE* file) const;
	bool WriteUdi(FILE* file) const;
	bool WriteTd0(FILE* file) const;

protected:
	qword	motor;	// 0 - not spinning, >0 - time when it'll stop
	int		cyl;
	int		side;
	int		ts_byte; // cpu.t per byte
	bool	write_protect;
	eUdi*	disk;
};

#endif//__FDD_H__
