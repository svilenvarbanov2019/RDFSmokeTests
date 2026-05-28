#include "pch.h"
#include <codecvt>

#define REG_CHARS_FILE_NAME "PutGetRegionalChars.ifc"


static const wchar_t*   TEST_WCHAR      = L"'English'\\ Русский'";
static const char*      TEST_WIN1251    = "'English'\\ \xD0\xF3\xF1\xF1\xEA\xE8\xE9'";
static const char*      TEST_STEP       = R"(''English''\\ \X2\0420\X0\\X2\0443\X0\\X2\0441\X0\\X2\0441\X0\\X2\043A\X0\\X2\0438\X0\\X2\0439\X0\'')";
static const wchar_t*   TEST_WCHAR_FORUTF8 = L"''English''\\\\ Русский''";

static const wchar_t* CHINESE_WCHAR = L"Chinese: 中国人";
static const char* CHINESE_WIN1251 = "Chinese: ???";
static const char* CHINESE_STEP = R"(Chinese: \X2\4E2D\X0\\X2\56FD\X0\\X2\4EBA\X0\)";

static const char* SLASH_ANSI = "\\";
static const wchar_t* SLASH_WCHAR = L"\\";
static const char* SLASH_STEP = "\\\\";

static const wchar_t* GREEK_WCHAR = L"'αβγ\\";
static const char* GREEK_WIN1251 = "'???\\";
static const char* GREEK_ISO8859_7 = "'\xE1\xE2\xE3\\";
static const char* GREEK_STEP = R"(''\X2\03B103B203B3\X0\\\)";

static const wchar_t* AGER_WCHAR = L"Ärge'r";
static const char* AGER_STEP = R"(\S\Drge\'r)";
static const char* AGER_WIN1251 = "\xC4rge'r"; //this is to support old behavior

static const char* PS_STEP = R"(\PE\\S\*\S\U\S\b)";
static const wchar_t* PS_WCHAR = L"Њет";
static const char* PS_WIN1251 = "\x8C\xE5\xF2";

static const char* CAT_STEP = R"(\X4\0001F6380001F5960000044F\X0\)";
static wchar_t CAT_WCHAR[] = L"😸🖖я";
static const char* CAT_WIN1251 = "??\xFF";
static const char* CAT_UTF8 = "\xF0\x9F\x98\xB8\xF0\x9F\x96\x96\xD1\x8F";

static const wchar_t* MIX_WCHAR = L"潦o㼿ÿ";
static const char* MIX_STEP = R"(\X2\6F66\X0\o\X2\3F3F\X0\\X\FF)";
static const char* MIX_WIN1251 = "\x3F\x6F\x3F\xFF";  //"?o?я" - this is to support old \X\ behavior

static void CheckRegionalChars(SdaiModel ifcModel, SdaiInteger stepId);

static std::string utf16_to_utf8(const std::wstring& utf16_str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(utf16_str);
}


static void CheckAttr(SdaiInstance inst, const char* attr, const char* ansi, const wchar_t* unicode, const char* step, const char* utf8 = NULL)
{
    char* getStr = NULL;
    wchar_t* getWStr = NULL;

    auto ret = sdaiGetAttrBN(inst, attr, sdaiSTRING, &getStr);
    ASSERT(ret && !strcmp(getStr, ansi));

    ret = sdaiGetAttrBN(inst, attr, sdaiUNICODE, &getWStr);
    ASSERT(ret && !wcscmp(getWStr, unicode));

    ret = sdaiGetAttrBN(inst, attr, sdaiEXPRESSSTRING, &getStr);
    ASSERT(ret && !strcmp(getStr, step));

    auto model = sdaiGetInstanceModel(inst);
    engiSetStringEncoding(model, enum_string_encoding::UTF8);

    auto sutf8 = utf16_to_utf8(unicode);
    if (utf8) {
        sutf8 = utf8;
    }

    getStr = NULL;
    ret = sdaiGetAttrBN(inst, attr, sdaiSTRING, &getStr);
    ASSERT(ret && !strcmp(getStr, sutf8.c_str()));

    engiSetStringEncoding(model, enum_string_encoding::WINDOWS_1251);
}

static void CheckHeader(SdaiModel ifcModel, int_t subitem, const char* ansi, const wchar_t* unicode, const char* step)
{
    const char* str = NULL;
    GetSPFFHeaderItem(ifcModel, 0, subitem, sdaiSTRING, &str);
    ASSERT(!strcmp(str, ansi));

    const wchar_t* wstr = NULL;
    GetSPFFHeaderItem(ifcModel, 0, subitem, sdaiUNICODE, &wstr);
    ASSERT(!wcscmp(wstr, unicode));

    str = NULL;
    GetSPFFHeaderItem(ifcModel, 0, subitem, sdaiEXPRESSSTRING, &str);
    ASSERT(!strcmp(str, step));

}

static void CheckHeader(SdaiModel ifcModel)
{
    CheckHeader(ifcModel, 0, TEST_WIN1251, TEST_WCHAR, TEST_STEP);
    CheckHeader(ifcModel, 1, TEST_WIN1251, TEST_WCHAR, TEST_STEP);
    CheckHeader(ifcModel, 2, SLASH_ANSI, SLASH_WCHAR, SLASH_STEP);
    CheckHeader(ifcModel, 3, SLASH_ANSI, SLASH_WCHAR, SLASH_STEP);
}


static void EncodingAndFilter(SdaiModel model)
{
    struct Codepage
    {
        enum_string_encoding code;
        int64_t              bitflag;
    };

    Codepage codepages[] = {
                                                                                        //		14, 15, 16, 17, 18, 19
        {enum_string_encoding::IGNORE_DEFAULT			    , (0 + 0 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 0 * 32)},	//		 0   0   0   0   0   0	
        {enum_string_encoding::WINDOWS_1250				    , (0 + 0 * 2 + 1 * 4 + 0 * 8 + 0 * 16 + 0 * 32)},	//		 0   0   1   0   0   0	
        {enum_string_encoding::WINDOWS_1251				    , (0 + 0 * 2 + 0 * 4 + 1 * 8 + 0 * 16 + 0 * 32)},	//		 0   0   0   1   0   0	
        {enum_string_encoding::WINDOWS_1252				    , (0 + 0 * 2 + 1 * 4 + 1 * 8 + 0 * 16 + 0 * 32)},	//		 0   0   1   1   0   0	
        {enum_string_encoding::WINDOWS_1253				    , (0 + 0 * 2 + 0 * 4 + 0 * 8 + 1 * 16 + 0 * 32)},	//		 0   0   0   0   1   0	
        {enum_string_encoding::WINDOWS_1254				    , (0 + 0 * 2 + 1 * 4 + 0 * 8 + 1 * 16 + 0 * 32)},	//		 0   0   1   0   1   0	
        {enum_string_encoding::WINDOWS_1255				    , (0 + 0 * 2 + 0 * 4 + 1 * 8 + 1 * 16 + 0 * 32)},	//		 0   0   0   1   1   0	
        {enum_string_encoding::WINDOWS_1256				    , (0 + 0 * 2 + 1 * 4 + 1 * 8 + 1 * 16 + 0 * 32)},	//		 0   0   1   1   1   0	
        {enum_string_encoding::WINDOWS_1257				    , (0 + 0 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 1 * 32)},	//		 0   0   0   0   0   1	
        {enum_string_encoding::WINDOWS_1258				    , (0 + 0 * 2 + 1 * 4 + 0 * 8 + 0 * 16 + 1 * 32)},	//		 0   0   1   0   0   1	
        {enum_string_encoding::ISO8859_1					, (1 + 0 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 0 * 32)},	//		 1   0   0   0   0   0	
        {enum_string_encoding::ISO8859_2					, (1 + 0 * 2 + 1 * 4 + 0 * 8 + 0 * 16 + 0 * 32)},	//		 1   0   1   0   0   0	
        {enum_string_encoding::ISO8859_3					, (1 + 0 * 2 + 0 * 4 + 1 * 8 + 0 * 16 + 0 * 32)},	//		 1   0   0   1   0   0	
        {enum_string_encoding::ISO8859_4					, (1 + 0 * 2 + 1 * 4 + 1 * 8 + 0 * 16 + 0 * 32)},	//		 1   0   1   1   0   0	
        {enum_string_encoding::ISO8859_5					, (1 + 0 * 2 + 0 * 4 + 0 * 8 + 1 * 16 + 0 * 32)},	//		 1   0   0   0   1   0	
        {enum_string_encoding::ISO8859_6					, (1 + 0 * 2 + 1 * 4 + 0 * 8 + 1 * 16 + 0 * 32)},	//		 1   0   1   0   1   0	
        {enum_string_encoding::ISO8859_7					, (1 + 0 * 2 + 0 * 4 + 1 * 8 + 1 * 16 + 0 * 32)},	//		 1   0   0   1   1   0	
        {enum_string_encoding::ISO8859_8					, (1 + 0 * 2 + 1 * 4 + 1 * 8 + 1 * 16 + 0 * 32)},	//		 1   0   1   1   1   0	
        {enum_string_encoding::ISO8859_9					, (1 + 0 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 1 * 32)},	//		 1   0   0   0   0   1	
        {enum_string_encoding::ISO8859_10				    , (1 + 0 * 2 + 1 * 4 + 0 * 8 + 0 * 16 + 1 * 32)},	//		 1   0   1   0   0   1	
        {enum_string_encoding::ISO8859_11				    , (1 + 0 * 2 + 0 * 4 + 1 * 8 + 0 * 16 + 1 * 32)},	//		 1   0   0   1   0   1	
        {enum_string_encoding::ISO8859_13				    , (1 + 0 * 2 + 0 * 4 + 0 * 8 + 1 * 16 + 1 * 32)},	//		 1   0   0   0   1   1	
        {enum_string_encoding::ISO8859_14				    , (1 + 0 * 2 + 1 * 4 + 0 * 8 + 1 * 16 + 1 * 32)},	//		 1   0   1   0   1   1	
        {enum_string_encoding::ISO8859_15				    , (1 + 0 * 2 + 0 * 4 + 1 * 8 + 1 * 16 + 1 * 32)},	//		 1   0   0   1   1   1	
        {enum_string_encoding::ISO8859_16				    , (1 + 0 * 2 + 1 * 4 + 1 * 8 + 1 * 16 + 1 * 32)},	//		 1   0   1   1   1   1	
        {enum_string_encoding::MACINTOSH_CENTRAL_EUROPEAN   , (0 + 1 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 0 * 32)},	//		 0   1   0   0   0   0	
        {enum_string_encoding::SHIFT_JIS_X_213			    , (1 + 1 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 0 * 32)},	//		 1   1   0   0   0   0	
        {enum_string_encoding::UTF8			                , (1 + 1 * 2 + 0 * 4 + 0 * 8 + 0 * 16 + 1 * 32)}	//		 1   1   0   0   0   1	
    };
   //not implemented  /*ISO8859_12				*/ (1 + 0 * 2 + 1 * 4 + 1 * 8 + 0 * 16 + 1 * 32),	//		 1   0   1   1   0   1	

    int64_t encoding_mask = (0b111111) << 14;

    //
    // all
    //

    for (int i = 0; i < _countof(codepages); i++) {

        int64_t setfilter = codepages[i].bitflag;
        setfilter = setfilter << 14;

        setFilter(model, setfilter, encoding_mask);

        auto getfilter = getFilter(model, encoding_mask);
        ASSERT(setfilter == getfilter);
    }

    for (int i = 0; i < _countof(codepages); i++) {

        auto code = codepages[i].code;

        engiSetStringEncoding(model, code);

        auto getfilter = getFilter(model, encoding_mask);
        ASSERT((codepages[i].bitflag << 14) == getfilter);
    }    

    //
    // switch
    //

    int64_t filter_ISO8859_6 = (1 + 0 * 2 + 1 * 4 + 0 * 8 + 1 * 16 + 0 * 32); //		 1   0   1   0   1   0	
    int64_t filter_ISO8859_7 = (1 + 0 * 2 + 0 * 4 + 1 * 8 + 1 * 16 + 0 * 32); //		 1   0   0   1   1   0	

    filter_ISO8859_6 <<= 14;
    filter_ISO8859_7 <<= 14;

    setFilter(model, filter_ISO8859_6, encoding_mask);
    auto getfilter = getFilter(model, encoding_mask);
    ASSERT(filter_ISO8859_6 == getfilter);

    int64_t change_mask = 0b11;
    change_mask <<= 16;
    
    getfilter = getFilter(model, change_mask);
    ASSERT(getfilter == ((int64_t)(0b01)<<16));

    int64_t bit17 = 1;
    bit17 <<= 17;

    setFilter(model, bit17, change_mask);

    getfilter = getFilter(model, encoding_mask);
    ASSERT(filter_ISO8859_7 == getfilter);

    getfilter = getFilter(model, change_mask);
    ASSERT(getfilter == ((int64_t)(0b10) << 16));
}

static void EncodingAndFilter()
{
    SdaiModel  model = sdaiCreateModelBNUnicode(L"IFC4");
    ASSERT(model);

    EncodingAndFilter(model);

    sdaiCloseModel(model);

    EncodingAndFilter(NULL);
}

//
const int_t BLOCK_LENGTH_READ = 20000;  //  MAX: 65535
FILE* myFileRead = nullptr;
static int_t   __stdcall   ReadCallBackFunction(unsigned char* content)
{
    if (myFileRead == nullptr || feof(myFileRead)) {
        return  -1;
    }

    int_t   size = fread(content, 1, BLOCK_LENGTH_READ, myFileRead);

    return  size;
}

static void CheckRegionalChars(const char* stepFile, SdaiInteger stepId)
{
    auto ifcModel = sdaiOpenModelBN(0, stepFile, "IFC4");
    CheckRegionalChars(ifcModel, stepId);
    sdaiCloseModel(ifcModel);

    fopen_s(&myFileRead, stepFile, "rb");
    ifcModel = engiOpenModelByStream(0, &ReadCallBackFunction, "IFC4");
    fclose(myFileRead);
    CheckRegionalChars(ifcModel, stepId);
    sdaiCloseModel(ifcModel);

    unsigned char content[10240];
    int_t size = _countof(content);
    fopen_s(&myFileRead, stepFile, "rb");
    size = fread(content, 1, size, myFileRead);
    fclose(myFileRead);

    ifcModel = engiOpenModelByArray(0, content, size, "IFC4");
    CheckRegionalChars(ifcModel, stepId);
    sdaiCloseModel(ifcModel);
}

//
const int_t BLOCK_LENGTH_WRITE = 20000; //  no maximum limit
static FILE* myFileWrite = nullptr;
static void    __stdcall   WriteCallBackFunction(unsigned char* content, int_t size)
{
    fwrite(content, (size_t)size, 1, myFileWrite);
}

static void CheckRegionalChars(SdaiModel ifcModel, SdaiInteger stepId)
{
    engiSetStringEncoding(ifcModel, enum_string_encoding::WINDOWS_1251);

    CheckHeader(ifcModel);

    auto wall = internalGetInstanceFromP21Line(ifcModel, stepId);
    CheckAttr(wall, "Name", TEST_WIN1251, TEST_WCHAR, TEST_STEP);
    CheckAttr(wall, "Description", TEST_WIN1251, TEST_WCHAR, TEST_STEP);

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 1);
    CheckAttr(wall, "Name", SLASH_ANSI, SLASH_WCHAR, SLASH_STEP);
    CheckAttr(wall, "Description", SLASH_ANSI, SLASH_WCHAR, SLASH_STEP);

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 2);
    CheckAttr(wall, "Name", CHINESE_WIN1251, CHINESE_WCHAR, CHINESE_STEP);
    CheckAttr(wall, "Description", GREEK_WIN1251, GREEK_WCHAR, GREEK_STEP);
    engiSetStringEncoding(ifcModel, enum_string_encoding::ISO8859_7);
    CheckAttr(wall, "Description", GREEK_ISO8859_7, GREEK_WCHAR, GREEK_STEP);
    engiSetStringEncoding(ifcModel, enum_string_encoding::WINDOWS_1251);

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 3);
    CheckAttr(wall, "Name", AGER_WIN1251, AGER_WCHAR, AGER_STEP);
    CheckAttr(wall, "Description", PS_WIN1251, PS_WCHAR, PS_STEP);

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 4);
    CheckAttr(wall, "Name", CAT_WIN1251, CAT_WCHAR, CAT_STEP, CAT_UTF8);
    CheckAttr(wall, "Description", MIX_WIN1251, MIX_WCHAR, MIX_STEP);

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 5);
    auto test = utf16_to_utf8(TEST_WCHAR_FORUTF8);
    CheckAttr(wall, "Name", TEST_WIN1251, TEST_WCHAR, test.c_str());
    auto chiness = utf16_to_utf8(CHINESE_WCHAR);
    CheckAttr(wall, "Description", CHINESE_WIN1251, CHINESE_WCHAR, chiness.c_str());

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 6);
    CheckAttr(wall, "Name", CAT_WIN1251, CAT_WCHAR, CAT_UTF8, CAT_UTF8);
    auto mix = utf16_to_utf8(MIX_WCHAR);
    CheckAttr(wall, "Description", MIX_WIN1251, MIX_WCHAR, mix.c_str());

    wall = internalGetInstanceFromP21Line(ifcModel, stepId + 7);
    auto step = utf16_to_utf8(TEST_WCHAR_FORUTF8);
    CheckAttr(wall, "Name", TEST_WIN1251, TEST_WCHAR, step.c_str());
    CheckAttr(wall, "Description", CAT_WIN1251, CAT_WCHAR, CAT_UTF8, CAT_UTF8);
}


static void PutGetRegionalChars(void)
{
    SdaiModel  ifcModel = sdaiCreateModelBNUnicode(L"IFC4");
    ASSERT(ifcModel);

    setFilter(ifcModel, 131072, ((int64_t)0b111111)<<14);

    SetSPFFHeaderItem(ifcModel, 0, 0, sdaiSTRING, TEST_WIN1251);
    SetSPFFHeaderItem(ifcModel, 0, 1, sdaiUNICODE, TEST_WCHAR);
    SetSPFFHeaderItem(ifcModel, 0, 2, sdaiSTRING, SLASH_ANSI);
    SetSPFFHeaderItem(ifcModel, 0, 3, sdaiUNICODE, SLASH_WCHAR);

    //
    auto wall = IFC4::IfcWall::Create(ifcModel);
    auto stepId = internalGetP21Line(wall);
    sdaiPutAttrBN(wall, "Name", sdaiSTRING, TEST_WIN1251);
    sdaiPutAttrBN(wall, "Description", sdaiUNICODE, TEST_WCHAR);

    engiSetComment(wall, "Should we support regional characters in comment?");

    //
    wall = IFC4::IfcWall::Create(ifcModel);
    sdaiPutAttrBN(wall, "Name", sdaiSTRING, SLASH_ANSI);
    sdaiPutAttrBN(wall, "Description", sdaiUNICODE, SLASH_WCHAR);


    //
    wall = IFC4::IfcWall::Create(ifcModel);
    sdaiPutAttrBN(wall, "Name", sdaiUNICODE, CHINESE_WCHAR);
    sdaiPutAttrBN(wall, "Description", sdaiEXPRESSSTRING, GREEK_STEP);

    //
    wall = IFC4::IfcWall::Create(ifcModel);
    sdaiPutAttrBN(wall, "Name", sdaiEXPRESSSTRING, AGER_STEP);
    sdaiPutAttrBN(wall, "Description", sdaiEXPRESSSTRING, PS_STEP);

    //
    wall = IFC4::IfcWall::Create(ifcModel);
    sdaiPutAttrBN(wall, "Name", sdaiEXPRESSSTRING, CAT_STEP);
    sdaiPutAttrBN(wall, "Description", sdaiUNICODE, MIX_WCHAR);

    //UTF8
    wall = IFC4::IfcWall::Create(ifcModel);
    auto test = utf16_to_utf8(TEST_WCHAR_FORUTF8);
    sdaiPutAttrBN(wall, "Name", sdaiEXPRESSSTRING, test.c_str());
    auto chiness = utf16_to_utf8(CHINESE_WCHAR);
    sdaiPutAttrBN(wall, "Description", sdaiEXPRESSSTRING, chiness.c_str());

    wall = IFC4::IfcWall::Create(ifcModel);
    sdaiPutAttrBN(wall, "Name", sdaiEXPRESSSTRING, CAT_UTF8);
    auto mix = utf16_to_utf8(MIX_WCHAR);
    sdaiPutAttrBN(wall, "Description", sdaiEXPRESSSTRING, mix.c_str());

    wall = IFC4::IfcWall::Create(ifcModel);
    engiSetStringEncoding(ifcModel, enum_string_encoding::UTF8);
    test = utf16_to_utf8(TEST_WCHAR);
    sdaiPutAttrBN(wall, "Name", sdaiSTRING, test.c_str());
    sdaiPutAttrBN(wall, "Description", sdaiSTRING, CAT_UTF8);

    /////////////////
    CheckRegionalChars(ifcModel, stepId);
    
    //
    sdaiSaveModelBN(ifcModel, "sdaiSaveModelBN_" REG_CHARS_FILE_NAME);
    CheckRegionalChars("sdaiSaveModelBN_" REG_CHARS_FILE_NAME, stepId);

    //
    fopen_s(&myFileWrite, "engiSaveModelByStream_" REG_CHARS_FILE_NAME, "wb");
    engiSaveModelByStream(ifcModel, WriteCallBackFunction, BLOCK_LENGTH_WRITE);
    fclose(myFileWrite);

    CheckRegionalChars("engiSaveModelByStream_" REG_CHARS_FILE_NAME, stepId);

    //
    unsigned char content[10240];
    int_t size = _countof(content);
    engiSaveModelByArray(ifcModel, content, &size);

    fopen_s(&myFileWrite, "engiSaveModelByArray_" REG_CHARS_FILE_NAME, "wb");
    fwrite(content, 1, size, myFileWrite);
    fclose(myFileWrite);

    CheckRegionalChars("engiSaveModelByArray_" REG_CHARS_FILE_NAME, stepId);

    //
    //sdaiSaveModelAsXmlBN(ifcModel, "sdaiSaveModelAsXmlBN.xml");
    //TODO - XML issues
    // CheckRegionalChars("sdaiSaveModelAsXmlBN.xml", stepId);

    //sdaiSaveModelAsSimpleXmlBN(ifcModel, "sdaiSaveModelAsSimpleXmlBN.xml");
    //TODO - XML issues
    //CheckRegionalChars("sdaiSaveModelAsSimpleXmlBN.xml", stepId);

    sdaiCloseModel(ifcModel);

}

static void InvalidTextLiterals()
{
    auto ifcModel = sdaiOpenModelBN(0, "..\\TestData\\InvalidTextLiterals.ifc", "");
    ASSERT(ifcModel);

    int nissues = 0;
    auto valres = validateModel(ifcModel);
    auto issue = validateGetFirstIssue(valres);
    while (issue) {
        nissues++;
        ASSERT(validateGetIssueType(issue) == enum_validation_type::__INVALID_TEXT_LITERAL);
        issue = validateGetNextIssue(issue);
    }
    ASSERT(nissues == 22);
    validateFreeResults(valres);

    for (int i = 2; i < 26; i++) {
        auto inst = internalGetInstanceFromP21Line(ifcModel, i);
        wchar_t* w = NULL;
        sdaiGetAttrBN(inst, "Name", sdaiUNICODE, &w);
        char* v = NULL;
        sdaiGetAttrBN(inst, "Name", sdaiSTRING, &v);
    }

    sdaiCloseModel(ifcModel);
}


extern void Encodings(void)
{
    ENTER_TEST;

    if (sizeof(wchar_t) == 4) {
        CAT_WCHAR[0] = (wchar_t)0x0001f638;
        CAT_WCHAR[1] = (wchar_t)0x0001f596;
    }

    EncodingAndFilter();
    PutGetRegionalChars();
    InvalidTextLiterals();

}