/*
 *	language.h -- some language settings (assembler syntax).
 *	Copyright (C) 2013 Gregoire Liglet and Florent Chiron, University of Kent.
 *	Copyright (C) 2013-2014 Fred Barnes, University of Kent <frmb@kent.ac.uk>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QSet>
#include <QString>
#include <QStringList>
#include <QMultiMap>

typedef struct OpcodeInfo
{
	QString name;
	QString parameters;
	QString description;
	QStringList operations;
	QString flags;
	QString nClocks;
} OpcodeInfo;

typedef struct DirectiveInfo
{
	QString name;
	QString parameters;
	QString description;
	QString example;
} DirectiveInfo;

extern const QMultiMap<QString, OpcodeInfo> OpcodesInfo;
extern const QMap<QString, DirectiveInfo> DirectivesInfo;

static const QSet<QString> AVRASMKeywords = {
	"r0",
	"r1",
	"r2",
	"r3",
	"r4",
	"r5",
	"r6",
	"r7",
	"r8",
	"r9",
	"r10",
	"r11",
	"r12",
	"r13",
	"r14",
	"r15",
	"r16",
	"r17",
	"r18",
	"r19",
	"r20",
	"r21",
	"r22",
	"r23",
	"r24",
	"r25",
	"r26",
	"r27",
	"r28",
	"r29",
	"r30",
	"r31",
	"add",
	"adc",
	"adiw",
	"sub",
	"subi",
	"sbc",
	"sbci",
	"sbiw",
	"and",
	"andi",
	"or",
	"ori",
	"eor",
	"com",
	"neg",
	"sbr",
	"cbr",
	"inc",
	"dec",
	"tst",
	"clr",
	"ser",
	"mul",
	"muls",
	"mulsu",
	"fmul",
	"fmuls",
	"fmulsu",
	"rjmp",
	"ijmp",
	"eijmp",
	"jmp",
	"rcall",
	"icall",
	"eicall",
	"call",
	"ret",
	"reti",
	"cpse",
	"cp",
	"cpc",
	"cpi",
	"sbrc",
	"sbrs",
	"sbic",
	"sbis",
	"brbs",
	"brbc",
	"breq",
	"brne",
	"brcs",
	"brcc",
	"brsh",
	"brlo",
	"brmi",
	"brpl",
	"brge",
	"brlt",
	"brhs",
	"brhc",
	"brts",
	"brtc",
	"brvs",
	"brvc",
	"brie",
	"brid",
	"mov",
	"movw",
	"ldi",
	"lds",
	"ld",
	"ldd",
	"sts",
	"st",
	"std",
	"lpm",
	"elpm",
	"spm",
	"in",
	"out",
	"push",
	"pop",
	"lsl",
	"lsr",
	"rol",
	"ror",
	"asr",
	"swap",
	"bset",
	"bclr",
	"sbi",
	"cbi",
	"bst",
	"bld",
	"sec",
	"clc",
	"sen",
	"cln",
	"sez",
	"clz",
	"sei",
	"cli",
	"ses",
	"cls",
	"sev",
	"clv",
	"set",
	"clt",
	"seh",
	"clh",
	"break",
	"nop",
	"sleep",
	"wdr",
	"org",
	"equ",
	"def",
	"include",
	"text",
	"data",
	"section",
	"eeprom",
	"const",
	"const16",
	"macro",
	"endmacro",
	"target",
	"mcu",
	"space",
	"space16",
	"hi",
	"lo",
	"X",
	"Y",
	"Z",
	"hi2",
	"hi3",
	"function",
	"endfunction",
	"let",
	"dload",
	"dstore",
	"signed",
	"unsigned",
	"if",
	"else",
	"elsif",
	"endif",
	"using"
};

static const QSet<QString> AVRASMSymbols = {
	",",
	".",
	":",
	"=",
	"+",
	"-",
	"*",
	"/",
	"%",
	"&",
	"|",
	"^",
	"(",
	")",
	"[",
	"]",
	"<<",
	">>",
	"?",
	":",
	"~",
	"==",
	"{",
	"}"
};

#endif // LANGUAGE_H
