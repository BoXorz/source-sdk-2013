"Resource/UI/MSInventoryMenu.res"
{
	"InventoryMenu"
	{
		"ControlName"		"CMSInventoryMenu"
		"fieldName"		"InventoryMenu"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"855"
		"tall"			"480"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
	}
	"TitleBGImg"
	{
		"ControlName"		"ImagePanel"
		"xpos"			"c-415"
		"ypos"			"c-230"
		"zpos"			"0"
		"wide"			"1400"
		"tall"			"540"
		"tileImage"		"0"
		"scaleImage"		"1"
		"image"			"bookmenu"
		"border"			"border"
	}
	"LPageTitleLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"LPageTitleLabel"
		"xpos"		"c-290"
		"ypos"		"25"
		"zpos"		"1"		
		"wide"		"260"
		"tall"		"28"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"0"
		"tabPosition"		"0"
		"disabledfgcolor2_override"	"InkWellLight"
		"textAlignment"		"center"
		"labelText"		"#MSS_INVENTORY"
		"font"			"HeaderFont22"		
	}	
	"RPageTitleLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"RPageTitleLabel"
		"xpos"		"c12"
		"ypos"		"25"
		"zpos"		"1"		
		"wide"		"340"
		"tall"		"28"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"0"
		"tabPosition"		"0"
		"disabledfgcolor2_override"	"InkWellLight"
		"textAlignment"		"center"
		"labelText"		"#ERROR"
		"font"			"HeaderFont22"		
	}	
	"LPageNumLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"LPageNumLabel"
		"xpos"		"c-190"
		"ypos"		"424"
		"zpos"		"1"
		"wide"		"60"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"0"
		"tabPosition"		"0"
		"disabledfgcolor2_override"	"InkWellLight"
		"labelText"		"3"
		"font"			"HeaderFont34"		
		"textAlignment"		"center"
	}	
	"RPageNumLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"RPageNumLabel"
		"xpos"		"c160"
		"ypos"		"424"
		"zpos"		"1"
		"wide"		"60"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"0"
		"tabPosition"		"0"
		"disabledfgcolor2_override"	"InkWellLight"
		"labelText"		"4"	
		"font"			"HeaderFont34"		
		"textAlignment"		"center"
	}
	"LPageTurnButton"
	{
		"ControlName"		"Button"
		"fieldName"		"LPageTurnButton"
		"xpos"		"c-340"
		"ypos"		"424"
		"wide"		"160"
		"tall"		"28"
		"autoResize"		"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"font"			"WritingFontVerySmall"
		"defaultFgColor_override"	"InkWellLight"
		"armedBgColor_override"	"Blank"
		"labelText"		"#MSS_TURNPAGE"
		"textAlignment"		"center"
		"command"		"TurnPageLeft"
		"Default"		"1"
	}	
	"RPageTurnButton"
	{
		"ControlName"		"Button"
		"fieldName"		"RPageTurnButton"
		"xpos"		"c210"
		"ypos"		"424"
		"wide"		"160"
		"tall"		"28"
		"autoResize"		"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"font"			"WritingFontVerySmall"
		"defaultFgColor_override"	"InkWellLight"
		"armedBgColor_override"	"Blank"
		"labelText"		"#MSS_TURNPAGE"
		"textAlignment"		"center"
		"command"		"TurnPageRight"
		"Default"		"1"
	}
	
	"GoldText"
	{
		"ControlName"		"Label"
		"fieldName"		"GoldText"
		"xpos"		"c40"
		"ypos"		"400"
		"zpos"		"1"		
		"wide"		"60"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
//		"disabledfgcolor2_override"	"InkWellLight"
		"textAlignment"		"east"
		"labelText"		"#MSS_GOLD"
		"font"			"WritingFontSmall"
//		"TextColor"		"RedInk"
	}
	"GoldLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"GoldLabel"
		"xpos"		"c110"
		"ypos"		"400"
		"zpos"		"1"		
		"wide"		"40"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
//		"disabledfgcolor2_override"	"InkWellLight"
		"textAlignment"		"west"
		"labelText"		"#ERROR"
		"font"			"WritingFontSmall"
//		"TextColor"		"RedInk"
	}	
}
