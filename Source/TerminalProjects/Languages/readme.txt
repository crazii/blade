/************************************************************************/
/* strings                                                                     */
/************************************************************************/
You can use BTLang("text") directly without define the macro BLANG_TEXT

Defining a string macro in *_blang.h is optional, its useful for engine libraries.
But It's recommended to use BTLang("") directly in the source code for convenience.


/************************************************************************/
/* Images, Audios                                                                     */
/************************************************************************/
put locale dependent resources in the image/, audio/ folder.
the related makefiles are located in Textures/Audio project, named as Locale[.*].makefile


/************************************************************************/
/* remarks                                                                     */
/************************************************************************/
some string/words are auto translated, i.e. Dialog first class child text,  option check box, option pure string, option drop down, menus.
use BXLang("") to force adding the word the lang table, so it can be auto-translated.