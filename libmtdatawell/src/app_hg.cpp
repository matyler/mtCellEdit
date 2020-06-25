/*
	Copyright (C) 2019 Mark Tyler

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

#include "core.h"



mtDW::Homoglyph::Homoglyph ()
{
	add_root_nodes ( "!ǃ﹗！" );
	add_root_nodes ( "\"ʺ˝ˮ“”‟″‶＂" );
	add_root_nodes ( "#﹟＃" );
	add_root_nodes ( "$﹩＄" );
	add_root_nodes ( "%﹪％" );
	add_root_nodes ( "&﹠＆" );
	add_root_nodes ( "'ʹʻʼʽˈˊˋ՚՛՝᾽᾿`´῾‘’‛′‵ꞌ＇" );
	add_root_nodes ( "(﹙（" );
	add_root_nodes ( ")﹚）" );
	add_root_nodes ( "*⁎⁕∗﹡＊" );
	add_root_nodes ( "+﹢＋" );
	add_root_nodes ( ",ˏꓹ﹐，" );
	add_root_nodes ( "-‐‑‒–—―−﹣－" );
	add_root_nodes ( ".ꓸ﹒．" );
	add_root_nodes ( "/∕／" );
	add_root_nodes ( "0０𝟎𝟢𝟬𝟶" );
	add_root_nodes ( "1１𝟏𝟣𝟭𝟷" );
	add_root_nodes ( "2２𝟐𝟤𝟮𝟸" );
	add_root_nodes ( "3ƷǮǯȜȝʒЗзҘҙӞӟӠӡ３𝟑𝟥𝟯𝟹" );
	add_root_nodes ( "4４𝟒𝟦𝟰𝟺" );
	add_root_nodes ( "5Ƽƽ５𝟓𝟧𝟱𝟻" );
	add_root_nodes ( "6６𝟔𝟨𝟲𝟼" );
	add_root_nodes ( "7７𝟕𝟩𝟳𝟽" );
	add_root_nodes ( "8８𝟖𝟪𝟴𝟾" );
	add_root_nodes ( "9Ꝯꝯ９𝟗𝟫𝟵𝟿" );
	add_root_nodes ( ":ː˸։⁚∶ꓽ꞉﹕：" );
	add_root_nodes ( ";⁏ꓼ﹔；" );
	add_root_nodes ( "<‹﹤＜" );
	add_root_nodes ( "=꓿꞊﹦＝" );
	add_root_nodes ( ">›﹥＞" );
	add_root_nodes ( "?﹖？" );
	add_root_nodes ( "@﹫＠" );
	add_root_nodes ( "AÀÁÂÃÄÅĀĂĄǍǞǠǺȀȂȦΆΑАӐӒᎪḀẠẢẤẦẨẪẬẮẰẲẴẶἈἉἊἋἌἍἎἏᾈᾉᾊᾋᾌᾍᾎᾏᾸᾹᾺΆᾼÅꓮＡ𝐀𝐴𝑨𝓐𝖠𝗔𝘈𝘼𝙰𝚨𝛢𝜜𝝖𝞐🄐🄰🅐🅰" );
	add_root_nodes ( "BƁɃʙΒВвᏴḂḄḆꓐＢ𝐁𝐵𝑩𝓑𝖡𝗕𝘉𝘽𝙱𝚩𝛣𝜝𝝗𝞑🄑🄱🅑🅱" );
	add_root_nodes ( "CÇĆĈĊČƇϹϾСҪᏟḈ℃ⅭꓚＣ𝐂𝐶𝑪𝓒𝖢𝗖𝘊𝘾𝙲🄒🄲🅒🅲" );
	add_root_nodes ( "DÐĎĐƉƊᎠḊḌḎḐḒⅮꓓＤ𝐃𝐷𝑫𝓓𝖣𝗗𝘋𝘿𝙳🄓🄳🅓🅳" );
	add_root_nodes ( "EÈÉÊËĒĔĖĘĚȄȆȨΈΕЀЁЕӖᎬḔḖḘḚḜẸẺẼẾỀỂỄỆἘἙἚἛἜἝῈΈꓰＥ𝐄𝐸𝑬𝓔𝖤𝗘𝘌𝙀𝙴𝚬𝛦𝜠𝝚𝞔🄔🄴🅔🅴" );
	add_root_nodes ( "FƑϜҒḞ℉ꓝꜰＦ𝐅𝐹𝑭𝓕𝖥𝗙𝘍𝙁𝙵🄕🄵🅕🅵" );
	add_root_nodes ( "GĜĞĠĢƓǤǦǴɢʛԌᏀḠꓖＧ𝐆𝐺𝑮𝓖𝖦𝗚𝘎𝙂𝙶🄖🄶🅖🅶" );
	add_root_nodes ( "HĤĦȞʜΉΗНнҢҤҥӇӉӈӊᎻḢḤḦḨḪἨἩἪἫἬἭἮἯᾘᾙᾚᾛᾜᾝᾞᾟῊΉῌⱧꓧꞪＨ𝐇𝐻𝑯𝓗𝖧𝗛𝘏𝙃𝙷𝚮𝛨𝜢𝝜𝞖🄗🄷🅗🅷" );
	add_root_nodes ( "IÌÍÎÏĨĪĬĮİƗǏȈȊΊΙΪІЇӀӏḬḮỈỊἸἹἺἻἼἽἾἿῘῙῚΊⅠꓲＩ𝐈𝐼𝑰𝓘𝖨𝗜𝘐𝙄𝙸𝚰𝛪𝜤𝝞𝞘🄘🄸🅘🅸" );
	add_root_nodes ( "JĴɈͿ̋ЈᎫꓙＪ𝐉𝐽𝑱𝓙𝖩𝗝𝘑𝙅𝙹🄙🄹🅙🅹" );
	add_root_nodes ( "KĶƘǨΚЌКҚҞҠᏦḰḲḴKⱩꓗꝀꝂꝄＫ𝐊𝐾𝑲𝓚𝖪𝗞𝘒𝙆𝙺𝚱𝛫𝜥𝝟𝞙🄚🄺🅚🅺" );
	add_root_nodes ( "LĹĻĽĿŁȽʟᏞḶḸḺḼⅬⱢꓡꝈＬ𝐋𝐿𝑳𝓛𝖫𝗟𝘓𝙇𝙻🄛🄻🅛🅻" );
	add_root_nodes ( "MΜМмӍᎷḾṀṂⅯⱮꓟＭ𝐌𝑀𝑴𝓜𝖬𝗠𝘔𝙈𝙼𝚳𝛭𝜧𝝡𝞛🄜🄼🅜🅼" );
	add_root_nodes ( "NÑŃŅŇƝǸɴΝṄṆṈṊꓠꞐＮ𝐍𝑁𝑵𝓝𝖭𝗡𝘕𝙉𝙽𝚴𝛮𝜨𝝢𝞜🄝🄽🅝🅽" );
	add_root_nodes ( "OÒÓÔÕÖŌŎŐƠǑǪǬȌȎȪȬȮȰΌΟϘОӦՕᎤᏅṌṎṐṒỌỎỐỒỔỖỘỚỜỞỠỢὈὉὊὋὌὍῸΌꓳꝹＯ𝐎𝑂𝑶𝓞𝖮𝗢𝘖𝙊𝙾𝚶𝛰𝜪𝝤𝞞🄞🄾🅞🅾" );
	add_root_nodes ( "PƤΡРᏢṔṖῬꓑꝐꝒＰ𝐏𝑃𝑷𝓟𝖯𝗣𝘗𝙋𝙿𝚸𝛲𝜬𝝦𝞠🄟🄿🅟🅿" );
	add_root_nodes ( "QԚꝖＱ𝐐𝑄𝑸𝓠𝖰𝗤𝘘𝙌𝚀🄠🅀🅠🆀" );
	add_root_nodes ( "RŔŖŘȐȒɌʀᎡᏒṘṚṜṞꓣＲ𝐑𝑅𝑹𝓡𝖱𝗥𝘙𝙍𝚁🄡🅁🅡🆁" );
	add_root_nodes ( "SŚŜŞŠȘЅՏᏚṠṢṤṦṨⱾꓢꜱＳ𝐒𝑆𝑺𝓢𝖲𝗦𝘚𝙎𝚂🄢🅂🅢🆂" );
	add_root_nodes ( "TŢŤŦƬƮȚΤТтҬҭᎢṪṬṮṰꓔＴ𝐓𝑇𝑻𝓣𝖳𝗧𝘛𝙏𝚃𝚻𝛵𝜯𝝩𝞣🄣🅃🅣🆃" );
	add_root_nodes ( "UÙÚÛÜŨŪŬŮŰŲƯǓǕǗǙǛȔȖՍṲṴṶṸṺỤỦỨỪỬỮỰꓴＵ𝐔𝑈𝑼𝓤𝖴𝗨𝘜𝙐𝚄🄤🅄🅤🆄" );
	add_root_nodes ( "VѴѶᏙṼṾⅤꓦＶ𝐕𝑉𝑽𝓥𝖵𝗩𝘝𝙑𝚅🄥🅅🅥🆅" );
	add_root_nodes ( "WŴԜᎳᏔẀẂẄẆẈⱲꓪＷ𝐖𝑊𝑾𝓦𝖶𝗪𝘞𝙒𝚆🄦🅆🅦🆆" );
	add_root_nodes ( "XΧХҲӼẊẌⅩꓫＸ𝐗𝑋𝑿𝓧𝖷𝗫𝘟𝙓𝚇𝚾𝛸𝜲𝝬𝞦🄧🅇🅧🆇" );
	add_root_nodes ( "YÝŶŸƳȲʏΎΥΫҮҰẎỲỴỶỸὙὛὝὟῨῩῪΎꓬＹ𝐘𝑌𝒀𝓨𝖸𝗬𝘠𝙔𝚈𝚼𝛶𝜰𝝪𝞤🄨🅈🅨🆈" );
	add_root_nodes ( "ZŹŻŽƵȤΖᏃẐẒẔⱫꓜＺ𝐙𝑍𝒁𝓩𝖹𝗭𝘡𝙕𝚉𝚭𝛧𝜡𝝛𝞕🄩🅉🅩🆉" );
	add_root_nodes ( "[［" );
	add_root_nodes ( "\\﹨＼" );
	add_root_nodes ( "]］" );
	add_root_nodes ( "^˄ˆ＾" );
	add_root_nodes ( "_ˍ＿" );
	add_root_nodes ( "`｀" );
	add_root_nodes ( "aàáâãäåāăąǎǟǡǻȁȃȧаӑӓḁạẚảấầẩẫậắằẳẵặａ𝐚𝑎𝒂𝓪𝔞𝖆𝖺𝗮𝘢𝙖𝚊" );
	add_root_nodes ( "bƀƄƅɓᏏḃḅḇｂ𝐛𝑏𝒃𝓫𝔟𝖇𝖻𝗯𝘣𝙗𝚋" );
	add_root_nodes ( "cçćĉċčƈϲсҫḉⅽｃ𝐜𝑐𝒄𝓬𝔠𝖈𝖼𝗰𝘤𝙘𝚌" );
	add_root_nodes ( "dďđɖɗԀԁḋḍḏḑḓⅾꓒｄ𝐝𝑑𝒅𝓭𝔡𝖉𝖽𝗱𝘥𝙙𝚍" );
	add_root_nodes ( "eèéêëēĕėęěȅȇȩеѐёҼҽҾҿӗḕḗḙḛḝẹẻẽếềểễệℯｅ𝐞𝑒𝒆𝓮𝔢𝖊𝖾𝗲𝘦𝙚𝚎" );
	add_root_nodes ( "fƒϝғḟｆ𝐟𝑓𝒇𝓯𝔣𝖋𝖿𝗳𝘧𝙛𝚏" );
	add_root_nodes ( "gĝğġģǥǧǵɠɡցḡｇ𝐠𝑔𝒈𝓰𝔤𝖌𝗀𝗴𝘨𝙜𝚐" );
	add_root_nodes ( "hĥħȟɦћҺһհᏂḣḥḧḩḫℎⱨｈ𝐡𝒉𝓱𝔥𝖍𝗁𝗵𝘩𝙝𝚑" );
	add_root_nodes ( "iìíîïĩīĭįıǐȉȋɨіїᎥḭḯỉịⅰｉ𝐢𝑖𝒊𝓲𝔦𝖎𝗂𝗶𝘪𝙞𝚒" );
	add_root_nodes ( "jĵǰɉɟϳјｊ𝐣𝑗𝒋𝓳𝔧𝖏𝗃𝗷𝘫𝙟𝚓" );
	add_root_nodes ( "kķƙǩкќқҟҡḱḳḵⱪꝁꝃꝅｋ𝐤𝑘𝒌𝓴𝔨𝖐𝗄𝗸𝘬𝙠𝚔𝛋𝜅𝜿𝝹𝞳" );
	add_root_nodes ( "lĺļľŀłƚɫɭḷḹḻḽⅼｌ𝐥𝑙𝒍𝓵𝔩𝖑𝗅𝗹𝘭𝙡𝚕" );
	add_root_nodes ( "mӎḿṁṃⅿｍ𝐦𝑚𝒎𝓶𝔪𝖒𝗆𝗺𝘮𝙢𝚖" );
	add_root_nodes ( "nñńņňŉƞǹɳոṅṇṉṋꞑｎ𝐧𝑛𝒏𝓷𝔫𝖓𝗇𝗻𝘯𝙣𝚗𝛈𝜂𝜼𝝶𝞰" );
	add_root_nodes ( "oòóôõöōŏőơǒǫǭȍȏȫȭȯȱοόϙоӧօṍṏṑṓọỏốồổỗộớờởỡợὀὁὂὃὄὅὸόꝺｏ𝐨𝑜𝒐𝓸𝔬𝖔𝗈𝗼𝘰𝙤𝚘𝛐𝜊𝝄𝝾𝞸" );
	add_root_nodes ( "pрṕṗꝑꝓｐ𝐩𝑝𝒑𝓹𝔭𝖕𝗉𝗽𝘱𝙥𝚙𝛒𝜌𝝆𝞀𝞺" );
	add_root_nodes ( "qԛզꝗｑ𝐪𝑞𝒒𝓺𝔮𝖖𝗾𝘲𝙦𝚚" );
	add_root_nodes ( "rŕŗřȑȓɍɼṙṛṝṟｒ𝐫𝑟𝒓𝓻𝔯𝖗𝗋𝗿𝘳𝙧𝚛" );
	add_root_nodes ( "sśŝşšșȿѕṡṣṥṧṩｓ𝐬𝑠𝒔𝓼𝔰𝖘𝗌𝘀𝘴𝙨𝚜" );
	add_root_nodes ( "tţťŧțṫṭṯṱẗｔ𝐭𝑡𝒕𝓽𝔱𝖙𝗍𝘁𝘵𝙩𝚝" );
	add_root_nodes ( "uùúûüũūŭůűųưǔǖǘǚǜȕȗսṳṵṷṹṻụủứừửữựｕ𝐮𝑢𝒖𝓾𝔲𝖚𝗎𝘂𝘶𝙪𝚞" );
	add_root_nodes ( "vѵѷṽṿⅴ∨ⱱｖ𝐯𝑣𝒗𝓿𝔳𝖛𝗏𝘃𝘷𝙫𝚟𝛎𝜈𝝊𝝼𝞶" );
	add_root_nodes ( "wŵԝẁẃẅẇẉẘⱳｗ𝐰𝑤𝒘𝔀𝔴𝖜𝗐𝘄𝘸𝙬𝚠" );
	add_root_nodes ( "xхҳӽẋẍⅹｘ𝐱𝑥𝒙𝔁𝔵𝖝𝗑𝘅𝘹𝙭𝚡" );
	add_root_nodes ( "yýÿŷȳуўүұӮӯӰӱӲӳᎩẏẙỳỵỷỹｙ𝐲𝑦𝒚𝔂𝗒𝘆𝘺𝙮𝚢" );
	add_root_nodes ( "zźżžȥɀẑẓẕⱬｚ𝐳𝑧𝒛𝔃𝗓𝘇𝘻𝙯𝚣" );
	add_root_nodes ( "{﹛｛" );
	add_root_nodes ( "|∣｜" );
	add_root_nodes ( "}﹜｝" );
	add_root_nodes ( "~˜῀⁓∼～" );
}

