/*
	Copyright (C) 2019-2020 Mark Tyler

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

#include "app_hg.h"



static int const FLIST_COLS = 4;



static char const * const s_font_list[mtDW::Utf8Font::TYPE_MAX +1][FLIST_COLS] =
{
{ "ASCII",		"ABCDEFGHIJKLMNOPQRSTUVWXYZ", "abcdefghijklmnopqrstuvwxyz", "0123456789"	},
{ "Bold",		"𝐀𝐁𝐂𝐃𝐄𝐅𝐆𝐇𝐈𝐉𝐊𝐋𝐌𝐍𝐎𝐏𝐐𝐑𝐒𝐓𝐔𝐕𝐖𝐗𝐘𝐙", "𝐚𝐛𝐜𝐝𝐞𝐟𝐠𝐡𝐢𝐣𝐤𝐥𝐦𝐧𝐨𝐩𝐪𝐫𝐬𝐭𝐮𝐯𝐰𝐱𝐲𝐳", "𝟎𝟏𝟐𝟑𝟒𝟓𝟔𝟕𝟖𝟗"	},
{ "Italic",		"𝐴𝐵𝐶𝐷𝐸𝐹𝐺𝐻𝐼𝐽𝐾𝐿𝑀𝑁𝑂𝑃𝑄𝑅𝑆𝑇𝑈𝑉𝑊𝑋𝑌𝑍", "𝑎𝑏𝑐𝑑𝑒𝑓𝑔ℎ𝑖𝑗𝑘𝑙𝑚𝑛𝑜𝑝𝑞𝑟𝑠𝑡𝑢𝑣𝑤𝑥𝑦𝑧",	NULL },
{ "Bold Italic",	"𝑨𝑩𝑪𝑫𝑬𝑭𝑮𝑯𝑰𝑱𝑲𝑳𝑴𝑵𝑶𝑷𝑸𝑹𝑺𝑻𝑼𝑽𝑾𝑿𝒀𝒁",	"𝒂𝒃𝒄𝒅𝒆𝒇𝒈𝒉𝒊𝒋𝒌𝒍𝒎𝒏𝒐𝒑𝒒𝒓𝒔𝒕𝒖𝒗𝒘𝒙𝒚𝒛",	NULL },
{ "Script",		"𝒜ℬ𝒞𝒟ℰℱ𝒢ℋℐ𝒥𝒦ℒℳ𝒩𝒪𝒫𝒬ℛ𝒮𝒯𝒰𝒱𝒲𝒳𝒴𝒵", "𝒶𝒷𝒸𝒹ℯ𝒻ℊ𝒽𝒾𝒿𝓀𝓁𝓂𝓃ℴ𝓅𝓆𝓇𝓈𝓉𝓊𝓋𝓌𝓍𝓎𝓏", NULL },
{ "Script Bold",	"𝓐𝓑𝓒𝓓𝓔𝓕𝓖𝓗𝓘𝓙𝓚𝓛𝓜𝓝𝓞𝓟𝓠𝓡𝓢𝓣𝓤𝓥𝓦𝓧𝓨𝓩", "𝓪𝓫𝓬𝓭𝓮𝓯𝓰𝓱𝓲𝓳𝓴𝓵𝓶𝓷𝓸𝓹𝓺𝓻𝓼𝓽𝓾𝓿𝔀𝔁𝔂𝔃",	NULL },
{ "Fraktur",		"𝔄𝔅ℭ𝔇𝔈𝔉𝔊ℌℑ𝔍𝔎𝔏𝔐𝔑𝔒𝔓𝔔ℜ𝔖𝔗𝔘𝔙𝔚𝔛𝔜ℨ", "𝔞𝔟𝔠𝔡𝔢𝔣𝔤𝔥𝔦𝔧𝔨𝔩𝔪𝔫𝔬𝔭𝔮𝔯𝔰𝔱𝔲𝔳𝔴𝔵𝔶𝔷", NULL },
{ "Fraktur Bold",	"𝕬𝕭𝕮𝕯𝕰𝕱𝕲𝕳𝕴𝕵𝕶𝕷𝕸𝕹𝕺𝕻𝕼𝕽𝕾𝕿𝖀𝖁𝖂𝖃𝖄𝖅", "𝖆𝖇𝖈𝖉𝖊𝖋𝖌𝖍𝖎𝖏𝖐𝖑𝖒𝖓𝖔𝖕𝖖𝖗𝖘𝖙𝖚𝖛𝖜𝖝𝖞𝖟", NULL },
{ "Sans",		"𝖠𝖡𝖢𝖣𝖤𝖥𝖦𝖧𝖨𝖩𝖪𝖫𝖬𝖭𝖮𝖯𝖰𝖱𝖲𝖳𝖴𝖵𝖶𝖷𝖸𝖹", "𝖺𝖻𝖼𝖽𝖾𝖿𝗀𝗁𝗂𝗃𝗄𝗅𝗆𝗇𝗈𝗉𝗊𝗋𝗌𝗍𝗎𝗏𝗐𝗑𝗒𝗓", "𝟢𝟣𝟤𝟥𝟦𝟧𝟨𝟩𝟪𝟫" },
{ "Sans Bold",		"𝗔𝗕𝗖𝗗𝗘𝗙𝗚𝗛𝗜𝗝𝗞𝗟𝗠𝗡𝗢𝗣𝗤𝗥𝗦𝗧𝗨𝗩𝗪𝗫𝗬𝗭", "𝗮𝗯𝗰𝗱𝗲𝗳𝗴𝗵𝗶𝗷𝗸𝗹𝗺𝗻𝗼𝗽𝗾𝗿𝘀𝘁𝘂𝘃𝘄𝘅𝘆𝘇", "𝟬𝟭𝟮𝟯𝟰𝟱𝟲𝟳𝟴𝟵" },
{ "Sans Italic",	"𝘈𝘉𝘊𝘋𝘌𝘍𝘎𝘏𝘐𝘑𝘒𝘓𝘔𝘕𝘖𝘗𝘘𝘙𝘚𝘛𝘜𝘝𝘞𝘟𝘠𝘡", "𝘢𝘣𝘤𝘥𝘦𝘧𝘨𝘩𝘪𝘫𝘬𝘭𝘮𝘯𝘰𝘱𝘲𝘳𝘴𝘵𝘶𝘷𝘸𝘹𝘺𝘻", NULL },
{ "Sans Bold Italic",	"𝘼𝘽𝘾𝘿𝙀𝙁𝙂𝙃𝙄𝙅𝙆𝙇𝙈𝙉𝙊𝙋𝙌𝙍𝙎𝙏𝙐𝙑𝙒𝙓𝙔𝙕", "𝙖𝙗𝙘𝙙𝙚𝙛𝙜𝙝𝙞𝙟𝙠𝙡𝙢𝙣𝙤𝙥𝙦𝙧𝙨𝙩𝙪𝙫𝙬𝙭𝙮𝙯", NULL },
{ "Monospace",		"𝙰𝙱𝙲𝙳𝙴𝙵𝙶𝙷𝙸𝙹𝙺𝙻𝙼𝙽𝙾𝙿𝚀𝚁𝚂𝚃𝚄𝚅𝚆𝚇𝚈𝚉", "𝚊𝚋𝚌𝚍𝚎𝚏𝚐𝚑𝚒𝚓𝚔𝚕𝚖𝚗𝚘𝚙𝚚𝚛𝚜𝚝𝚞𝚟𝚠𝚡𝚢𝚣", "𝟶𝟷𝟸𝟹𝟺𝟻𝟼𝟽𝟾𝟿" },
{ "Double Strike",	"𝔸𝔹ℂ𝔻𝔼𝔽𝔾ℍ𝕀𝕁𝕂𝕃𝕄ℕ𝕆ℙℚℝ𝕊𝕋𝕌𝕍𝕎𝕏𝕐ℤ", "𝕒𝕓𝕔𝕕𝕖𝕗𝕘𝕙𝕚𝕛𝕜𝕝𝕞𝕟𝕠𝕡𝕢𝕣𝕤𝕥𝕦𝕧𝕨𝕩𝕪𝕫", "𝟘𝟙𝟚𝟛𝟜𝟝𝟞𝟟𝟠𝟡" }

// NOTE: must be same order as TYPE_* in header file.
// https://en.wikipedia.org/wiki/Mathematical_Alphanumeric_Symbols

// NOTE: When this table changes, remember to re-run the code below to get
// the add ( xxxxx ); code for runtime.
};



mtDW::Utf8Font::Utf8Font ()
{
#ifdef DEBUG
	for ( int i = TYPE_ASCII; i <= TYPE_MAX; i++ )
	{
		// Validate that the UTF-8 size matches the ASCII version
		for ( int j = 1; j < FLIST_COLS; j++ )
		{
			if ( ! s_font_list[i][j] )
			{
				continue;
			}

			if ( 1 != mtkit_utf8_string_legal (
				(unsigned char const *)s_font_list[i][j], 0 )
				)
			{
				std::cerr << "mtDW::Utf8Font::Utf8Font"
					<< " Illegal UTF-8 string : "
					<< s_font_list[i][j]
					<< " i=" << i
					<< " j=" << j
					<< "\n";
				throw 123;
			}

			size_t const a_len = strlen ( s_font_list[0][j] );
			size_t const b_len = (size_t)mtkit_utf8_len (
				(unsigned char const *)s_font_list[i][j], 0 );

			if ( a_len != b_len )
			{
				std::cerr << "mtDW::Utf8Font::Utf8Font"
					<< " i=" << i
					<< " j=" << j
					<< " a_len=" << a_len
					<< " b_len=" << b_len
					<< "\n";
				throw 123;
			}
		}
	}
#endif

	// Note: this code is only needed once when the above table changes.
	// Paste the output into this source below to avoid runtime overheads.
#if 0==1
	for ( int i = 1; i < FLIST_COLS; i++ )
	{
		unsigned char const * src[ mtDW::Utf8Font::TYPE_MAX + 1 ]
			= { NULL };

		for ( int j = TYPE_ASCII; j <= TYPE_MAX; j++ )
		{
			if ( ! s_font_list[j][i] )
			{
				continue;
			}

			src[j] = (unsigned char const *)s_font_list[j][i];
		}

		size_t const len = strlen ( s_font_list[0][i] );

		for ( size_t g = 0; g < len; g++ )
		{
			std::string nodes;

			// Root ASCII node
			nodes += s_font_list[TYPE_ASCII][i][g];

			for ( int j = TYPE_ASCII + 1; j <= TYPE_MAX; j++ )
			{
				if ( ! src[j] )
				{
					continue;
				}

				int const glyph_len = mtkit_utf8_offset (
					src[j], 1 );

				if ( glyph_len < 1 )
				{
					std::cerr << "mtDW::Utf8Font::Utf8Font"
						<< " Unexpected end of string"
						<< " font #=" << j
						<< " glyph ="
						<< src[TYPE_ASCII][g]
						<< "\n";
				}

				std::string glyph ( src[j], (size_t)glyph_len );

				nodes += glyph;
				src[j] += glyph_len;
			}

			add_root_nodes ( nodes );
			std::cout << "\tadd_root_nodes ( \"" << nodes
				<< "\" );\n";
		}
	}
#else
	add_root_nodes ( "A𝐀𝐴𝑨𝒜𝓐𝔄𝕬𝖠𝗔𝘈𝘼𝙰𝔸" );
	add_root_nodes ( "B𝐁𝐵𝑩ℬ𝓑𝔅𝕭𝖡𝗕𝘉𝘽𝙱𝔹" );
	add_root_nodes ( "C𝐂𝐶𝑪𝒞𝓒ℭ𝕮𝖢𝗖𝘊𝘾𝙲ℂ" );
	add_root_nodes ( "D𝐃𝐷𝑫𝒟𝓓𝔇𝕯𝖣𝗗𝘋𝘿𝙳𝔻" );
	add_root_nodes ( "E𝐄𝐸𝑬ℰ𝓔𝔈𝕰𝖤𝗘𝘌𝙀𝙴𝔼" );
	add_root_nodes ( "F𝐅𝐹𝑭ℱ𝓕𝔉𝕱𝖥𝗙𝘍𝙁𝙵𝔽" );
	add_root_nodes ( "G𝐆𝐺𝑮𝒢𝓖𝔊𝕲𝖦𝗚𝘎𝙂𝙶𝔾" );
	add_root_nodes ( "H𝐇𝐻𝑯ℋ𝓗ℌ𝕳𝖧𝗛𝘏𝙃𝙷ℍ" );
	add_root_nodes ( "I𝐈𝐼𝑰ℐ𝓘ℑ𝕴𝖨𝗜𝘐𝙄𝙸𝕀" );
	add_root_nodes ( "J𝐉𝐽𝑱𝒥𝓙𝔍𝕵𝖩𝗝𝘑𝙅𝙹𝕁" );
	add_root_nodes ( "K𝐊𝐾𝑲𝒦𝓚𝔎𝕶𝖪𝗞𝘒𝙆𝙺𝕂" );
	add_root_nodes ( "L𝐋𝐿𝑳ℒ𝓛𝔏𝕷𝖫𝗟𝘓𝙇𝙻𝕃" );
	add_root_nodes ( "M𝐌𝑀𝑴ℳ𝓜𝔐𝕸𝖬𝗠𝘔𝙈𝙼𝕄" );
	add_root_nodes ( "N𝐍𝑁𝑵𝒩𝓝𝔑𝕹𝖭𝗡𝘕𝙉𝙽ℕ" );
	add_root_nodes ( "O𝐎𝑂𝑶𝒪𝓞𝔒𝕺𝖮𝗢𝘖𝙊𝙾𝕆" );
	add_root_nodes ( "P𝐏𝑃𝑷𝒫𝓟𝔓𝕻𝖯𝗣𝘗𝙋𝙿ℙ" );
	add_root_nodes ( "Q𝐐𝑄𝑸𝒬𝓠𝔔𝕼𝖰𝗤𝘘𝙌𝚀ℚ" );
	add_root_nodes ( "R𝐑𝑅𝑹ℛ𝓡ℜ𝕽𝖱𝗥𝘙𝙍𝚁ℝ" );
	add_root_nodes ( "S𝐒𝑆𝑺𝒮𝓢𝔖𝕾𝖲𝗦𝘚𝙎𝚂𝕊" );
	add_root_nodes ( "T𝐓𝑇𝑻𝒯𝓣𝔗𝕿𝖳𝗧𝘛𝙏𝚃𝕋" );
	add_root_nodes ( "U𝐔𝑈𝑼𝒰𝓤𝔘𝖀𝖴𝗨𝘜𝙐𝚄𝕌" );
	add_root_nodes ( "V𝐕𝑉𝑽𝒱𝓥𝔙𝖁𝖵𝗩𝘝𝙑𝚅𝕍" );
	add_root_nodes ( "W𝐖𝑊𝑾𝒲𝓦𝔚𝖂𝖶𝗪𝘞𝙒𝚆𝕎" );
	add_root_nodes ( "X𝐗𝑋𝑿𝒳𝓧𝔛𝖃𝖷𝗫𝘟𝙓𝚇𝕏" );
	add_root_nodes ( "Y𝐘𝑌𝒀𝒴𝓨𝔜𝖄𝖸𝗬𝘠𝙔𝚈𝕐" );
	add_root_nodes ( "Z𝐙𝑍𝒁𝒵𝓩ℨ𝖅𝖹𝗭𝘡𝙕𝚉ℤ" );
	add_root_nodes ( "a𝐚𝑎𝒂𝒶𝓪𝔞𝖆𝖺𝗮𝘢𝙖𝚊𝕒" );
	add_root_nodes ( "b𝐛𝑏𝒃𝒷𝓫𝔟𝖇𝖻𝗯𝘣𝙗𝚋𝕓" );
	add_root_nodes ( "c𝐜𝑐𝒄𝒸𝓬𝔠𝖈𝖼𝗰𝘤𝙘𝚌𝕔" );
	add_root_nodes ( "d𝐝𝑑𝒅𝒹𝓭𝔡𝖉𝖽𝗱𝘥𝙙𝚍𝕕" );
	add_root_nodes ( "e𝐞𝑒𝒆ℯ𝓮𝔢𝖊𝖾𝗲𝘦𝙚𝚎𝕖" );
	add_root_nodes ( "f𝐟𝑓𝒇𝒻𝓯𝔣𝖋𝖿𝗳𝘧𝙛𝚏𝕗" );
	add_root_nodes ( "g𝐠𝑔𝒈ℊ𝓰𝔤𝖌𝗀𝗴𝘨𝙜𝚐𝕘" );
	add_root_nodes ( "h𝐡ℎ𝒉𝒽𝓱𝔥𝖍𝗁𝗵𝘩𝙝𝚑𝕙" );
	add_root_nodes ( "i𝐢𝑖𝒊𝒾𝓲𝔦𝖎𝗂𝗶𝘪𝙞𝚒𝕚" );
	add_root_nodes ( "j𝐣𝑗𝒋𝒿𝓳𝔧𝖏𝗃𝗷𝘫𝙟𝚓𝕛" );
	add_root_nodes ( "k𝐤𝑘𝒌𝓀𝓴𝔨𝖐𝗄𝗸𝘬𝙠𝚔𝕜" );
	add_root_nodes ( "l𝐥𝑙𝒍𝓁𝓵𝔩𝖑𝗅𝗹𝘭𝙡𝚕𝕝" );
	add_root_nodes ( "m𝐦𝑚𝒎𝓂𝓶𝔪𝖒𝗆𝗺𝘮𝙢𝚖𝕞" );
	add_root_nodes ( "n𝐧𝑛𝒏𝓃𝓷𝔫𝖓𝗇𝗻𝘯𝙣𝚗𝕟" );
	add_root_nodes ( "o𝐨𝑜𝒐ℴ𝓸𝔬𝖔𝗈𝗼𝘰𝙤𝚘𝕠" );
	add_root_nodes ( "p𝐩𝑝𝒑𝓅𝓹𝔭𝖕𝗉𝗽𝘱𝙥𝚙𝕡" );
	add_root_nodes ( "q𝐪𝑞𝒒𝓆𝓺𝔮𝖖𝗊𝗾𝘲𝙦𝚚𝕢" );
	add_root_nodes ( "r𝐫𝑟𝒓𝓇𝓻𝔯𝖗𝗋𝗿𝘳𝙧𝚛𝕣" );
	add_root_nodes ( "s𝐬𝑠𝒔𝓈𝓼𝔰𝖘𝗌𝘀𝘴𝙨𝚜𝕤" );
	add_root_nodes ( "t𝐭𝑡𝒕𝓉𝓽𝔱𝖙𝗍𝘁𝘵𝙩𝚝𝕥" );
	add_root_nodes ( "u𝐮𝑢𝒖𝓊𝓾𝔲𝖚𝗎𝘂𝘶𝙪𝚞𝕦" );
	add_root_nodes ( "v𝐯𝑣𝒗𝓋𝓿𝔳𝖛𝗏𝘃𝘷𝙫𝚟𝕧" );
	add_root_nodes ( "w𝐰𝑤𝒘𝓌𝔀𝔴𝖜𝗐𝘄𝘸𝙬𝚠𝕨" );
	add_root_nodes ( "x𝐱𝑥𝒙𝓍𝔁𝔵𝖝𝗑𝘅𝘹𝙭𝚡𝕩" );
	add_root_nodes ( "y𝐲𝑦𝒚𝓎𝔂𝔶𝖞𝗒𝘆𝘺𝙮𝚢𝕪" );
	add_root_nodes ( "z𝐳𝑧𝒛𝓏𝔃𝔷𝖟𝗓𝘇𝘻𝙯𝚣𝕫" );
	add_root_nodes ( "0𝟎𝟢𝟬𝟶𝟘" );
	add_root_nodes ( "1𝟏𝟣𝟭𝟷𝟙" );
	add_root_nodes ( "2𝟐𝟤𝟮𝟸𝟚" );
	add_root_nodes ( "3𝟑𝟥𝟯𝟹𝟛" );
	add_root_nodes ( "4𝟒𝟦𝟰𝟺𝟜" );
	add_root_nodes ( "5𝟓𝟧𝟱𝟻𝟝" );
	add_root_nodes ( "6𝟔𝟨𝟲𝟼𝟞" );
	add_root_nodes ( "7𝟕𝟩𝟳𝟽𝟟" );
	add_root_nodes ( "8𝟖𝟪𝟴𝟾𝟠" );
	add_root_nodes ( "9𝟗𝟫𝟵𝟿𝟡" );
#endif
}

int mtDW::Utf8Font::file_encode (
	char	const * const	input_utf8,
	int		const	type,
	char	const * const	output_utf8,
	std::string		&info
	)
{
	info.clear ();

	if ( ! input_utf8 || ! output_utf8 )
	{
		info = "Bad argument";
		return 1;
	}

	FileOps fops ( this );

	if ( fops.load_input_file ( input_utf8 ) )
	{
		info = "Unable to load input UTF-8 file";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info += "Not UTF-8 input file";
		return 1;
	}

	if ( fops.open_output_file ( output_utf8 ) )
	{
		info = "Unable to open output file";
		return 1;
	}

	if ( fops.encode_utf8font_file ( type ) )
	{
		info = "Unable to encode file";
		return 1;
	}

	return 0;
}

int mtDW::Utf8Font::utf8_encode (
	std::string	const	&input,
	int		const	type,
	std::string		&info,
	std::string		&output
	)
{
	info.clear ();
	output.clear ();

	FileOps fops ( this );

	if ( fops.load_input_utf8 ( input ) )
	{
		info = "Unable to load input text";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info = "Not UTF-8 input text";
		return 1;
	}

	if ( fops.load_input_utf8 ( input ) )
	{
		info = "Unable to load input text";
		return 1;
	}

	if ( fops.open_output_mem () )
	{
		info = "Unable to open output mem";
		return 1;
	}

	if ( fops.encode_utf8font_file ( type ) )
	{
		info = "Unable to encode input text";
		return 1;
	}

	if ( fops.get_output_mem_utf8 ( output ) )
	{
		info = "Unable to create output UTF-8";
		return 1;
	}

	return 0;
}

int mtDW::Utf8Font::get_type_name (
	int	const	type,
	std::string	&name
	)
{
	if ( type >= TYPE_MIN && type <= TYPE_MAX )
	{
		name = s_font_list[type][0];
		return 0;		// Found
	}

	return 1;			// Not found
}

void mtDW::Utf8Font::get_font_list ( std::string &txt )
{
	txt.clear ();

	for ( int i = TYPE_ASCII; i <= TYPE_MAX; i++ )
	{
		char buf[32];

		snprintf ( buf, sizeof(buf), "%i - ", i );
		txt += buf;

		for ( int j = 0; j < FLIST_COLS; j++ )
		{
			if ( s_font_list[i][j] )
			{
				switch ( j )
				{
				case 0:			break;
				case 1:	txt += " = ";	break;
				default: txt += " + ";	break;
				}

				txt += s_font_list[i][j];
			}
		}

		txt += "\n";
	}
}

