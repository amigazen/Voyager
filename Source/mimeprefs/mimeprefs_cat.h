#ifndef MIMEPREFS_CAT_H
#define MIMEPREFS_CAT_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif


/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSG_OK 0
#define MSG_CANCEL 1
#define MSG_NO 2
#define MSG_YES 3
#define MSG_ERROR 4
#define MSG_APP_DESC 5
#define MSG_APP_GUIDENAME 6
#define MSG_APP_FAILED 7
#define MSG_WIN_TITLE 8
#define MSG_BT_SAVE 9
#define MSG_BT_USE 10
#define MSG_BT_CANCEL 11
#define MSG_BT_ADD 12
#define MSG_BT_DEL 13
#define MSG_BT_COPY 14
#define MSG_BT_TESTVIEW 15
#define MSG_MIME_E_TYPE 16
#define MSG_MIME_E_EXT 17
#define MSG_MIME_E_APP 18
#define MSG_MIME_E_STORE 19
#define MSG_MIME_ACTS_1 20
#define MSG_MIME_ACTS_2 21
#define MSG_MIME_ACTS_3 22
#define MSG_MIME_ACTS_4 23
#define MSG_MIME_LV1 24
#define MSG_MIME_LV2 25
#define MSG_MIME_LV3 26
#define MSG_MIME_LV4 27
#define MSG_MIME_LV5 28
#define MSG_MIME_LV6 29
#define MSG_MIME_LV_ACT1 30
#define MSG_MIME_LV_ACT2 31
#define MSG_MIME_LV_ACT3 32
#define MSG_MIME_LV_ACT4 33
#define MSG_MIME_LV_CLASS 34
#define MSG_MIME_LV_ASK 35
#define MSG_MIME_E_UCD 36
#define MSG_VIEWER 37
#define MSG_VIEWINFO 38
#define MSG_VIEW_USE_INTERNAL 39
#define MSG_VIEW_USE_STREAM 40
#define MSG_MENU_PROJECT 41
#define MSG_MENU_OPEN 42
#define MSG_MENU_SAVEAS 43
#define MSG_MENU_ABOUT 44
#define MSG_MENU_QUIT 45
#define MSG_MENU_EDIT 46
#define MSG_MENU_DEFAULTS 47
#define MSG_MENU_LASTSAVED 48
#define MSG_MENU_RESTORE 49
#define MSG_MENU_IMPORT_V 50
#define MSG_MENU_SETTINGS 51
#define MSG_MENU_CREATEICONS 52
#define MSG_MENU_MUI 53
#define MSG_MENU_TEST 54
#define MSG_SH_MIME_CLASS 55
#define MSG_SH_MIME_TYPE 56
#define MSG_SH_MIME_EXT 57
#define MSG_SH_MIME_ACT 58
#define MSG_SH_MIME_STORE 59
#define MSG_SH_MIME_INTERNAL 60
#define MSG_SH_MIME_STREAM 61
#define MSG_SH_MIME_APP 62
#define MSG_IMPORT_V 63
#define MSG_NOFILE 64
#define MSG_FR_LOAD 65
#define MSG_FR_SAVE 66
#define MSG_WINTEST_TITLE 67
#define MSG_WINTEST_FILE 68
#define MSG_SH_WINTEST_FILE 69
#define MSG_WINTEST_DO 70
#define MSG_WINTEST_T_HEADER 71
#define MSG_WINTEST_T_UNKNOWN 72
#define MSG_WINTEST_T_INFO 73
#define MSG_WINTEST_T_RUN 74
#define MSG_MIME_NOCLASS 75
#define MSG_MIME_NOCLASS_ASK 76
#define MSG_TESTVIEW_FILETITLE 77

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSG_OK_STR "OK"
#define MSG_CANCEL_STR "Cancel"
#define MSG_NO_STR "No"
#define MSG_YES_STR "Yes"
#define MSG_ERROR_STR "An error occured!"
#define MSG_APP_DESC_STR "Global media type handling preferences"
#define MSG_APP_GUIDENAME_STR "MimePrefs.guide"
#define MSG_APP_FAILED_STR "Internal error:\nFailed to create MUI Application Object\nMUI Error code %ld\nPlease report this bug via email immediately to\n<bugs@vapor.com>\nState your system configuration and, if possible,\nthe circumstances which caused the bug."
#define MSG_WIN_TITLE_STR "MIME Media Type Preferences"
#define MSG_BT_SAVE_STR "Save"
#define MSG_BT_USE_STR "Use"
#define MSG_BT_CANCEL_STR "Cancel"
#define MSG_BT_ADD_STR "Add"
#define MSG_BT_DEL_STR "Remove"
#define MSG_BT_COPY_STR "Copy"
#define MSG_BT_TESTVIEW_STR "Test 'View'"
#define MSG_MIME_E_TYPE_STR "MIME type:"
#define MSG_MIME_E_EXT_STR "Extensions:"
#define MSG_MIME_E_APP_STR "Application:"
#define MSG_MIME_E_STORE_STR "Directory:"
#define MSG_MIME_ACTS_1_STR "Ask user"
#define MSG_MIME_ACTS_2_STR "Save to disk"
#define MSG_MIME_ACTS_3_STR "View"
#define MSG_MIME_ACTS_4_STR "Save & View"
#define MSG_MIME_LV1_STR "\033uType"
#define MSG_MIME_LV2_STR "\033uExtensions"
#define MSG_MIME_LV3_STR "\033uAction"
#define MSG_MIME_LV4_STR "\033uDir"
#define MSG_MIME_LV5_STR "\033uViewer"
#define MSG_MIME_LV6_STR "\033uIS"
#define MSG_MIME_LV_ACT1_STR "Ask"
#define MSG_MIME_LV_ACT2_STR "Save"
#define MSG_MIME_LV_ACT3_STR "View"
#define MSG_MIME_LV_ACT4_STR "S&V"
#define MSG_MIME_LV_CLASS_STR "(class)"
#define MSG_MIME_LV_ASK_STR "(ask everytime)"
#define MSG_MIME_E_UCD_STR "Use class directory?"
#define MSG_VIEWER_STR "Viewer"
#define MSG_VIEWINFO_STR "\033c(Placeholders: %f filename, %p pubscreenname)"
#define MSG_VIEW_USE_INTERNAL_STR "Use internal viewer?"
#define MSG_VIEW_USE_STREAM_STR "PIPE Streaming?"
#define MSG_MENU_PROJECT_STR "Project"
#define MSG_MENU_OPEN_STR "O\000Open..."
#define MSG_MENU_SAVEAS_STR "A\000Save as..."
#define MSG_MENU_ABOUT_STR "?\000About..."
#define MSG_MENU_QUIT_STR "Q\000Quit"
#define MSG_MENU_EDIT_STR "Edit"
#define MSG_MENU_DEFAULTS_STR "D\000Reset to Defaults..."
#define MSG_MENU_LASTSAVED_STR "L\000Last Saved"
#define MSG_MENU_RESTORE_STR "R\000Restore"
#define MSG_MENU_IMPORT_V_STR "V\000Import old Voyager MIME prefs..."
#define MSG_MENU_SETTINGS_STR "Settings"
#define MSG_MENU_CREATEICONS_STR "I\000Create Icons?"
#define MSG_MENU_MUI_STR "M\000MUI Settings..."
#define MSG_MENU_TEST_STR "T\000Test..."
#define MSG_SH_MIME_CLASS_STR "General \033bclass\033n of media.\nThis forms the MIME media type\ntogether with the more specific\n\033btype\033n string."
#define MSG_SH_MIME_TYPE_STR "Specific \033btype\033n of media. This forms the\nMIME media type together with the\n\033bclass\033n.\n\nYou can specify a default behavior for a\nwhole class by specifying \"*\" here."
#define MSG_SH_MIME_EXT_STR "File extensions which identify a specific\nMIME media type. For example, GIF pictures\ngenerally have the extension \".GIF\", thus\nyou would enter \".GIF\" here for the MIME type\n\"image/gif\".\n\nYou can specify multiple file extensions\ndelimited by spaces, in case there are\nseveral standard extensions for certain type\nof files (for example, \".html\" and \".htm\".)"
#define MSG_SH_MIME_ACT_STR "The action you want to take place\nwhen an applications encounters\na media of the specific type.\n\n\"View\" here only means that the media\nfile is processed with some application,\nwhich can be an player or viewer of any\nkind, a ARexx script or whatever you like."
#define MSG_SH_MIME_STORE_STR "A directory where you want files\nof the specific MIME type to be stored.\nYou can also select to store all files of a\nclass in the same directory.\n\nIf you leave this blank, applications\nwill ask you everytime when you are\ndownloading a file of the specific type."
#define MSG_SH_MIME_INTERNAL_STR "If an application has a internal viewer\nfor a certain media type, use this in\npreference to the viewing app specified above.\n\nYou should still define an external viewer\nfor applications which don't have a internal\nviewer (for example, Voyager can internally\ndisplay GIF pictures, AmIRC can't.)"
#define MSG_SH_MIME_STREAM_STR "Feed the media data to the viewing application\nusing streaming. This is only possible with some\nviewers, and may require special options."
#define MSG_SH_MIME_APP_STR "The \"Viewer\" application for this media type.\nIn addition to the pathname, you can specify\nplaceholders:\n? %f will be replaced with the filename of\n  media file to be viewed\n??%p will be replaced with the public screen\n  name of the application which processes\n  the media file"
#define MSG_IMPORT_V_STR "Import Voyager MIME preferences"
#define MSG_NOFILE_STR "\033cUnable to open file\n\033b%s\033n\n\nReason: %s (%ld)"
#define MSG_FR_LOAD_STR "Load MIME prefs preset..."
#define MSG_FR_SAVE_STR "Save MIME prefs preset..."
#define MSG_WINTEST_TITLE_STR "MIMEPrefs ??Testing window"
#define MSG_WINTEST_FILE_STR "File to test:"
#define MSG_SH_WINTEST_FILE_STR "Here you can drop or select a file\nto test your MIME prefs with.\nThis only works for files with extensions."
#define MSG_WINTEST_DO_STR "Test"
#define MSG_WINTEST_T_HEADER_STR "File: '%s'\n\n"
#define MSG_WINTEST_T_UNKNOWN_STR "Sorry, cannot identify file by extension."
#define MSG_WINTEST_T_INFO_STR "Type: %s%s\nAction: %s\n"
#define MSG_WINTEST_T_RUN_STR "Viewing with '%s'"
#define MSG_MIME_NOCLASS_STR "You have no class wide specification for files\nof class \033b%s*\033n. Do you want MIMEPrefs\nto create such an entry for you now?"
#define MSG_MIME_NOCLASS_ASK_STR "*\033bYes|No"
#define MSG_TESTVIEW_FILETITLE_STR "Select a file to test with:"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSG_OK,(STRPTR)MSG_OK_STR},
    {MSG_CANCEL,(STRPTR)MSG_CANCEL_STR},
    {MSG_NO,(STRPTR)MSG_NO_STR},
    {MSG_YES,(STRPTR)MSG_YES_STR},
    {MSG_ERROR,(STRPTR)MSG_ERROR_STR},
    {MSG_APP_DESC,(STRPTR)MSG_APP_DESC_STR},
    {MSG_APP_GUIDENAME,(STRPTR)MSG_APP_GUIDENAME_STR},
    {MSG_APP_FAILED,(STRPTR)MSG_APP_FAILED_STR},
    {MSG_WIN_TITLE,(STRPTR)MSG_WIN_TITLE_STR},
    {MSG_BT_SAVE,(STRPTR)MSG_BT_SAVE_STR},
    {MSG_BT_USE,(STRPTR)MSG_BT_USE_STR},
    {MSG_BT_CANCEL,(STRPTR)MSG_BT_CANCEL_STR},
    {MSG_BT_ADD,(STRPTR)MSG_BT_ADD_STR},
    {MSG_BT_DEL,(STRPTR)MSG_BT_DEL_STR},
    {MSG_BT_COPY,(STRPTR)MSG_BT_COPY_STR},
    {MSG_BT_TESTVIEW,(STRPTR)MSG_BT_TESTVIEW_STR},
    {MSG_MIME_E_TYPE,(STRPTR)MSG_MIME_E_TYPE_STR},
    {MSG_MIME_E_EXT,(STRPTR)MSG_MIME_E_EXT_STR},
    {MSG_MIME_E_APP,(STRPTR)MSG_MIME_E_APP_STR},
    {MSG_MIME_E_STORE,(STRPTR)MSG_MIME_E_STORE_STR},
    {MSG_MIME_ACTS_1,(STRPTR)MSG_MIME_ACTS_1_STR},
    {MSG_MIME_ACTS_2,(STRPTR)MSG_MIME_ACTS_2_STR},
    {MSG_MIME_ACTS_3,(STRPTR)MSG_MIME_ACTS_3_STR},
    {MSG_MIME_ACTS_4,(STRPTR)MSG_MIME_ACTS_4_STR},
    {MSG_MIME_LV1,(STRPTR)MSG_MIME_LV1_STR},
    {MSG_MIME_LV2,(STRPTR)MSG_MIME_LV2_STR},
    {MSG_MIME_LV3,(STRPTR)MSG_MIME_LV3_STR},
    {MSG_MIME_LV4,(STRPTR)MSG_MIME_LV4_STR},
    {MSG_MIME_LV5,(STRPTR)MSG_MIME_LV5_STR},
    {MSG_MIME_LV6,(STRPTR)MSG_MIME_LV6_STR},
    {MSG_MIME_LV_ACT1,(STRPTR)MSG_MIME_LV_ACT1_STR},
    {MSG_MIME_LV_ACT2,(STRPTR)MSG_MIME_LV_ACT2_STR},
    {MSG_MIME_LV_ACT3,(STRPTR)MSG_MIME_LV_ACT3_STR},
    {MSG_MIME_LV_ACT4,(STRPTR)MSG_MIME_LV_ACT4_STR},
    {MSG_MIME_LV_CLASS,(STRPTR)MSG_MIME_LV_CLASS_STR},
    {MSG_MIME_LV_ASK,(STRPTR)MSG_MIME_LV_ASK_STR},
    {MSG_MIME_E_UCD,(STRPTR)MSG_MIME_E_UCD_STR},
    {MSG_VIEWER,(STRPTR)MSG_VIEWER_STR},
    {MSG_VIEWINFO,(STRPTR)MSG_VIEWINFO_STR},
    {MSG_VIEW_USE_INTERNAL,(STRPTR)MSG_VIEW_USE_INTERNAL_STR},
    {MSG_VIEW_USE_STREAM,(STRPTR)MSG_VIEW_USE_STREAM_STR},
    {MSG_MENU_PROJECT,(STRPTR)MSG_MENU_PROJECT_STR},
    {MSG_MENU_OPEN,(STRPTR)MSG_MENU_OPEN_STR},
    {MSG_MENU_SAVEAS,(STRPTR)MSG_MENU_SAVEAS_STR},
    {MSG_MENU_ABOUT,(STRPTR)MSG_MENU_ABOUT_STR},
    {MSG_MENU_QUIT,(STRPTR)MSG_MENU_QUIT_STR},
    {MSG_MENU_EDIT,(STRPTR)MSG_MENU_EDIT_STR},
    {MSG_MENU_DEFAULTS,(STRPTR)MSG_MENU_DEFAULTS_STR},
    {MSG_MENU_LASTSAVED,(STRPTR)MSG_MENU_LASTSAVED_STR},
    {MSG_MENU_RESTORE,(STRPTR)MSG_MENU_RESTORE_STR},
    {MSG_MENU_IMPORT_V,(STRPTR)MSG_MENU_IMPORT_V_STR},
    {MSG_MENU_SETTINGS,(STRPTR)MSG_MENU_SETTINGS_STR},
    {MSG_MENU_CREATEICONS,(STRPTR)MSG_MENU_CREATEICONS_STR},
    {MSG_MENU_MUI,(STRPTR)MSG_MENU_MUI_STR},
    {MSG_MENU_TEST,(STRPTR)MSG_MENU_TEST_STR},
    {MSG_SH_MIME_CLASS,(STRPTR)MSG_SH_MIME_CLASS_STR},
    {MSG_SH_MIME_TYPE,(STRPTR)MSG_SH_MIME_TYPE_STR},
    {MSG_SH_MIME_EXT,(STRPTR)MSG_SH_MIME_EXT_STR},
    {MSG_SH_MIME_ACT,(STRPTR)MSG_SH_MIME_ACT_STR},
    {MSG_SH_MIME_STORE,(STRPTR)MSG_SH_MIME_STORE_STR},
    {MSG_SH_MIME_INTERNAL,(STRPTR)MSG_SH_MIME_INTERNAL_STR},
    {MSG_SH_MIME_STREAM,(STRPTR)MSG_SH_MIME_STREAM_STR},
    {MSG_SH_MIME_APP,(STRPTR)MSG_SH_MIME_APP_STR},
    {MSG_IMPORT_V,(STRPTR)MSG_IMPORT_V_STR},
    {MSG_NOFILE,(STRPTR)MSG_NOFILE_STR},
    {MSG_FR_LOAD,(STRPTR)MSG_FR_LOAD_STR},
    {MSG_FR_SAVE,(STRPTR)MSG_FR_SAVE_STR},
    {MSG_WINTEST_TITLE,(STRPTR)MSG_WINTEST_TITLE_STR},
    {MSG_WINTEST_FILE,(STRPTR)MSG_WINTEST_FILE_STR},
    {MSG_SH_WINTEST_FILE,(STRPTR)MSG_SH_WINTEST_FILE_STR},
    {MSG_WINTEST_DO,(STRPTR)MSG_WINTEST_DO_STR},
    {MSG_WINTEST_T_HEADER,(STRPTR)MSG_WINTEST_T_HEADER_STR},
    {MSG_WINTEST_T_UNKNOWN,(STRPTR)MSG_WINTEST_T_UNKNOWN_STR},
    {MSG_WINTEST_T_INFO,(STRPTR)MSG_WINTEST_T_INFO_STR},
    {MSG_WINTEST_T_RUN,(STRPTR)MSG_WINTEST_T_RUN_STR},
    {MSG_MIME_NOCLASS,(STRPTR)MSG_MIME_NOCLASS_STR},
    {MSG_MIME_NOCLASS_ASK,(STRPTR)MSG_MIME_NOCLASS_ASK_STR},
    {MSG_TESTVIEW_FILETITLE,(STRPTR)MSG_TESTVIEW_FILETITLE_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] =
{
    "\x00\x00\x00\x00\x00\x04"
    MSG_OK_STR "\x00\x00"
    "\x00\x00\x00\x01\x00\x08"
    MSG_CANCEL_STR "\x00\x00"
    "\x00\x00\x00\x02\x00\x04"
    MSG_NO_STR "\x00\x00"
    "\x00\x00\x00\x03\x00\x04"
    MSG_YES_STR "\x00"
    "\x00\x00\x00\x04\x00\x12"
    MSG_ERROR_STR "\x00"
    "\x00\x00\x00\x05\x00\x28"
    MSG_APP_DESC_STR "\x00\x00"
    "\x00\x00\x00\x06\x00\x10"
    MSG_APP_GUIDENAME_STR "\x00"
    "\x00\x00\x00\x07\x00\xE6"
    MSG_APP_FAILED_STR "\x00"
    "\x00\x00\x00\x08\x00\x1C"
    MSG_WIN_TITLE_STR "\x00"
    "\x00\x00\x00\x09\x00\x06"
    MSG_BT_SAVE_STR "\x00\x00"
    "\x00\x00\x00\x0A\x00\x04"
    MSG_BT_USE_STR "\x00"
    "\x00\x00\x00\x0B\x00\x08"
    MSG_BT_CANCEL_STR "\x00\x00"
    "\x00\x00\x00\x0C\x00\x04"
    MSG_BT_ADD_STR "\x00"
    "\x00\x00\x00\x0D\x00\x08"
    MSG_BT_DEL_STR "\x00\x00"
    "\x00\x00\x00\x0E\x00\x06"
    MSG_BT_COPY_STR "\x00\x00"
    "\x00\x00\x00\x0F\x00\x0C"
    MSG_BT_TESTVIEW_STR "\x00"
    "\x00\x00\x00\x10\x00\x0C"
    MSG_MIME_E_TYPE_STR "\x00\x00"
    "\x00\x00\x00\x11\x00\x0C"
    MSG_MIME_E_EXT_STR "\x00"
    "\x00\x00\x00\x12\x00\x0E"
    MSG_MIME_E_APP_STR "\x00\x00"
    "\x00\x00\x00\x13\x00\x0C"
    MSG_MIME_E_STORE_STR "\x00\x00"
    "\x00\x00\x00\x14\x00\x0A"
    MSG_MIME_ACTS_1_STR "\x00\x00"
    "\x00\x00\x00\x15\x00\x0E"
    MSG_MIME_ACTS_2_STR "\x00\x00"
    "\x00\x00\x00\x16\x00\x06"
    MSG_MIME_ACTS_3_STR "\x00\x00"
    "\x00\x00\x00\x17\x00\x0C"
    MSG_MIME_ACTS_4_STR "\x00"
    "\x00\x00\x00\x18\x00\x08"
    MSG_MIME_LV1_STR "\x00\x00"
    "\x00\x00\x00\x19\x00\x0E"
    MSG_MIME_LV2_STR "\x00\x00"
    "\x00\x00\x00\x1A\x00\x0A"
    MSG_MIME_LV3_STR "\x00\x00"
    "\x00\x00\x00\x1B\x00\x06"
    MSG_MIME_LV4_STR "\x00"
    "\x00\x00\x00\x1C\x00\x0A"
    MSG_MIME_LV5_STR "\x00\x00"
    "\x00\x00\x00\x1D\x00\x06"
    MSG_MIME_LV6_STR "\x00\x00"
    "\x00\x00\x00\x1E\x00\x04"
    MSG_MIME_LV_ACT1_STR "\x00"
    "\x00\x00\x00\x1F\x00\x06"
    MSG_MIME_LV_ACT2_STR "\x00\x00"
    "\x00\x00\x00\x20\x00\x06"
    MSG_MIME_LV_ACT3_STR "\x00\x00"
    "\x00\x00\x00\x21\x00\x04"
    MSG_MIME_LV_ACT4_STR "\x00"
    "\x00\x00\x00\x22\x00\x08"
    MSG_MIME_LV_CLASS_STR "\x00"
    "\x00\x00\x00\x23\x00\x10"
    MSG_MIME_LV_ASK_STR "\x00"
    "\x00\x00\x00\x24\x00\x16"
    MSG_MIME_E_UCD_STR "\x00\x00"
    "\x00\x00\x00\x25\x00\x08"
    MSG_VIEWER_STR "\x00\x00"
    "\x00\x00\x00\x26\x00\x30"
    MSG_VIEWINFO_STR "\x00"
    "\x00\x00\x00\x27\x00\x16"
    MSG_VIEW_USE_INTERNAL_STR "\x00\x00"
    "\x00\x00\x00\x28\x00\x10"
    MSG_VIEW_USE_STREAM_STR "\x00"
    "\x00\x00\x00\x29\x00\x08"
    MSG_MENU_PROJECT_STR "\x00"
    "\x00\x00\x00\x2A\x00\x0A"
    MSG_MENU_OPEN_STR "\x00"
    "\x00\x00\x00\x2B\x00\x0E"
    MSG_MENU_SAVEAS_STR "\x00\x00"
    "\x00\x00\x00\x2C\x00\x0C"
    MSG_MENU_ABOUT_STR "\x00\x00"
    "\x00\x00\x00\x2D\x00\x08"
    MSG_MENU_QUIT_STR "\x00\x00"
    "\x00\x00\x00\x2E\x00\x06"
    MSG_MENU_EDIT_STR "\x00\x00"
    "\x00\x00\x00\x2F\x00\x18"
    MSG_MENU_DEFAULTS_STR "\x00\x00"
    "\x00\x00\x00\x30\x00\x0E"
    MSG_MENU_LASTSAVED_STR "\x00\x00"
    "\x00\x00\x00\x31\x00\x0A"
    MSG_MENU_RESTORE_STR "\x00"
    "\x00\x00\x00\x32\x00\x24"
    MSG_MENU_IMPORT_V_STR "\x00\x00"
    "\x00\x00\x00\x33\x00\x0A"
    MSG_MENU_SETTINGS_STR "\x00\x00"
    "\x00\x00\x00\x34\x00\x10"
    MSG_MENU_CREATEICONS_STR "\x00"
    "\x00\x00\x00\x35\x00\x12"
    MSG_MENU_MUI_STR "\x00"
    "\x00\x00\x00\x36\x00\x0A"
    MSG_MENU_TEST_STR "\x00"
    "\x00\x00\x00\x37\x00\x6C"
    MSG_SH_MIME_CLASS_STR "\x00"
    "\x00\x00\x00\x38\x00\xA6"
    MSG_SH_MIME_TYPE_STR "\x00"
    "\x00\x00\x00\x39\x01\x64"
    MSG_SH_MIME_EXT_STR "\x00"
    "\x00\x00\x00\x3A\x01\x04"
    MSG_SH_MIME_ACT_STR "\x00\x00"
    "\x00\x00\x00\x3B\x01\x06"
    MSG_SH_MIME_STORE_STR "\x00"
    "\x00\x00\x00\x3C\x01\x26"
    MSG_SH_MIME_INTERNAL_STR "\x00"
    "\x00\x00\x00\x3D\x00\x8A"
    MSG_SH_MIME_STREAM_STR "\x00"
    "\x00\x00\x00\x3E\x01\x16"
    MSG_SH_MIME_APP_STR "\x00"
    "\x00\x00\x00\x3F\x00\x20"
    MSG_IMPORT_V_STR "\x00"
    "\x00\x00\x00\x40\x00\x30"
    MSG_NOFILE_STR "\x00\x00"
    "\x00\x00\x00\x41\x00\x1A"
    MSG_FR_LOAD_STR "\x00"
    "\x00\x00\x00\x42\x00\x1A"
    MSG_FR_SAVE_STR "\x00"
    "\x00\x00\x00\x43\x00\x1C"
    MSG_WINTEST_TITLE_STR "\x00\x00"
    "\x00\x00\x00\x44\x00\x0E"
    MSG_WINTEST_FILE_STR "\x00"
    "\x00\x00\x00\x45\x00\x6C"
    MSG_SH_WINTEST_FILE_STR "\x00"
    "\x00\x00\x00\x46\x00\x06"
    MSG_WINTEST_DO_STR "\x00\x00"
    "\x00\x00\x00\x47\x00\x0E"
    MSG_WINTEST_T_HEADER_STR "\x00\x00"
    "\x00\x00\x00\x48\x00\x2A"
    MSG_WINTEST_T_UNKNOWN_STR "\x00"
    "\x00\x00\x00\x49\x00\x18"
    MSG_WINTEST_T_INFO_STR "\x00\x00"
    "\x00\x00\x00\x4A\x00\x12"
    MSG_WINTEST_T_RUN_STR "\x00"
    "\x00\x00\x00\x4B\x00\x7C"
    MSG_MIME_NOCLASS_STR "\x00"
    "\x00\x00\x00\x4C\x00\x0A"
    MSG_MIME_NOCLASS_ASK_STR "\x00"
    "\x00\x00\x00\x4D\x00\x1C"
    MSG_TESTVIEW_FILETITLE_STR "\x00"
};

#endif /* CATCOMP_BLOCK */


/****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};


#ifdef CATCOMP_CODE

STRPTR GetString(struct LocaleInfo *li, LONG stringNum)
{
LONG   *l;
UWORD  *w;
STRPTR  builtIn;

    l = (LONG *)CatCompBlock;

    while (*l != stringNum)
    {
        w = (UWORD *)((ULONG)l + 4);
        l = (LONG *)((ULONG)l + (ULONG)*w + 6);
    }
    builtIn = (STRPTR)((ULONG)l + 6);

#define XLocaleBase LocaleBase
#define LocaleBase li->li_LocaleBase
    
    if (LocaleBase)
        return(GetCatalogStr(li->li_Catalog,stringNum,builtIn));
#define LocaleBase XLocaleBase
#undef XLocaleBase

    return(builtIn);
}


#endif /* CATCOMP_CODE */


/****************************************************************************/


#endif /* MIMEPREFS_CAT_H */
