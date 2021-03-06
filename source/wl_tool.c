/*	华宇拼音输入法V6词库外部工具。
 *  华宇拼音输入法V6外部词库工具	V0.5	2007年5月10日
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <process.h>
#include <wordlib.h>
#include <utility.h>
#include <ci.h>
#include <config.h>
#include <icw.h>
#include <pim_resource.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>

//简体词到繁体词的转换常量
#define	JF_MAXSTRINGLEN			40		/* 词最大长度 */
#define	JF_SLOTMAXITEMS			10		/* 一个SLOT中最多10个冲突 */
#define	JF_HASHMULT				41		/* 散列乘数 */
#define	JF_HASHSLOTS			16384	/* 散列项数 */
#define	JF_BASEADDRESS			(sizeof(int) * JF_HASHSLOTS)	/* 跳过index的起始地址 */

int ExportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ci_count, TCHAR *err_file_name, int export_all, void *call_back);
int ImportWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name, void *call_back);
int CreateWordLibrary(const TCHAR *wordlib_file_name, const TCHAR *text_file_name, int *ok_count, int *err_count, TCHAR *err_file_name, void *call_back);

const TCHAR *usage =
	TEXT("华宇拼音输入法V6外部词库工具 V0.5 2007年5月10日\n")
	TEXT("\n")
	TEXT("1. 将词库文件导出成文本文件\n")
	TEXT("2. 将文本合并到一个词库文件\n")
	TEXT("3. 基于文本创建新的词库文件\n")
	TEXT("4. 基于j2f文件生成j2f转换数据文件\n")
	TEXT("5. 将V5的词库导入到用户词库中\n")
	TEXT("6. 测试新词获取以及删除功能\n")
	TEXT("\n")
	TEXT("[说明]  由于本工具可能处理较多数量的词条，因此采用命令行的方式，这种方\n")
	TEXT("式比带有UI界面的程序效果高很多。当操作存在错误的时候，请阅读err.log文件\n")
	TEXT("\n")
	TEXT("[命令行参数]\n")
	TEXT("wl_tool /Export  wordlib_file text_file\n")
	TEXT("        /Import  wordlib_file text_file\n")
	TEXT("        /Create  wordlib_file text_file\n")
	TEXT("        /GenJ2F  j2f_data_file j2f_text_file\n")
	TEXT("		 /Upgrade wordlib_file\n")
#if 0
	TEXT("        /TestNewWord\n")
#endif
	TEXT("\n")
	TEXT("/Export  将词库中的词条导出到文本文件。\n")
	TEXT("         如：wl_tool /Export syswl.uwl ci100w.txt\n")
	TEXT("\n")
	TEXT("/Import  将文本文件导入到词库中。\n")
	TEXT("         对于不合法的词条，不进行合并，输出到ci_err.txt中。\n")
	TEXT("         如：wl_tool /Import usr.uwl ci100w.txt\n")
	TEXT("\n")
	TEXT("/Create  通过文本文件创建新的词库。\n")
	TEXT("         对于不合法的词条，输出到err.log中。\n")
	TEXT("         如：wl_tool /Create new-wl.uwl ci100w.txt\n")
	TEXT("\n")
	TEXT("/GenJ2F  通过J2F文本文件，生成j2f.dat文件。用于\n")
	TEXT("         输入法简体词到繁体词的转换。\n")
	TEXT("         如：wl_tool /Genj2f j2f.dat j2f.txt\n")
	TEXT("\n")
	TEXT("/Upgrade 将V5的词库文件导入到用户词库中。\n")
	TEXT("         只对用户词汇进行本操作。\n")
	TEXT("         如：wl_tool /Upgrade userwl.dat")
	TEXT("\n")
#if 0
	TEXT("/TestNewWord 测试新词表。从中读取最新的词条（URL方式），再\n")
	TEXT("             进行删除操作。\n")
	TEXT("             如：wltool /Testnewword\n")
#endif
	TEXT("\n";)

TCHAR err_file_name[0x400];

/**	测试新词表
 */
int DoTestNewWordTable()
{
#if 0
	if (TestNewWordTable())
		return 0;
	fprintf(stderr, "新词表测试失败");
	return 1;
#else
	return 0;
#endif
}

/**	导出词库内容
 */
int DoExport(const TCHAR *text_file_name, const TCHAR *wordlib_file_name)
{
	int ci_count = 0;
	int ret;

	fprintf(stdout, "正在导出词库<%s>的词条...\n", wordlib_file_name);
	ret = ExportWordLibrary(wordlib_file_name, text_file_name, &ci_count, err_file_name, 1, 0);
	if (ret)
		fprintf(stdout, "导出成功, 词条数目:%d\n", ci_count);
	else
		fprintf(stdout, "导出失败, 请阅读err.log文件\n");

	return !ret;
}

/**	导入词库
 */
int DoImport(const TCHAR *text_file_name, const TCHAR *wordlib_file_name)
{
	int ok_count = 0, err_count = 0;
	int ret;

	fprintf(stdout, "正在向词库<%s>的导入词条...\n", wordlib_file_name);
	ret = ImportWordLibrary(wordlib_file_name, text_file_name, &ok_count, &err_count, err_file_name, 0);
	if (ret)
		fprintf(stdout, "导入成功词条数目:%d, 失败数目:%d\n", ok_count, err_count);
	else
		fprintf(stdout, "导入失败, 请阅读err.log文件\n");

	return !ret;
}

/**	创建词库
 */
int DoCreate(const TCHAR *text_file_name, const TCHAR *wordlib_file_name)
{
	int ok_count = 0, err_count = 0;
	int ret;

	fprintf(stdout, "正在创建词库<%S>...\n", wordlib_file_name);
	ret = CreateWordLibrary(wordlib_file_name, text_file_name, &ok_count, &err_count, err_file_name, 0);
	if (ret)
		fprintf(stdout, "创建成功, 成功词条数目:%d, 失败词条数目:%d\n", ok_count, err_count);
	else
		fprintf(stdout, "创建失败, 请阅读err.log文件\n");

	return !ret;
}
/**	获得汉字词串的哈希散列Key.
 *	注意: 在程序中不判断字符串的长度
 */
int GetHashKey(const TCHAR *string)
{
	unsigned int key = 0;

	while (*string)
		key = key * JF_HASHMULT + *string++;

	return (int)(key % JF_HASHSLOTS);
}

/**	在hash表项中增加字符串.
 *	参数：
 *		slot			将要放置到的Slot指针位置
 *		j_ci			简体词汇
 *		f_ci			繁体词汇
 *	返回：
 *		1：成功
 *		0：失败
 */
int AddSlotString(TCHAR **slot, TCHAR *j_ci, TCHAR *f_ci)
{
	TCHAR tmp[JF_MAXSTRINGLEN * 2 * 2 * JF_SLOTMAXITEMS] = {0};

	//如果Slot已经被占用，则将原来的串与本次的串合并
	//然后重新分配
	if (*slot)
	{
		_tcscpy_s(tmp, _SizeOf(tmp), *slot);
		free(*slot);
	}

	_tcscat_s(tmp, _SizeOf(tmp), j_ci);
	_tcscat_s(tmp, _SizeOf(tmp), TEXT(","));
	_tcscat_s(tmp, _SizeOf(tmp), f_ci);
	_tcscat_s(tmp, _SizeOf(tmp), TEXT("."));

	*slot = _tcsdup(tmp);
	if (!*slot)
		return 0;

	return 1;
}

/**	是否将简繁词汇加入到jf表中
 */
int NeedInsertJFCi(TCHAR *j_ci, TCHAR *f_ci)
{
	TCHAR j2f_ci[0x100] = {0};
	extern void StringJ2F(TCHAR *zi_string);

	//过长，不插入
	if (_tcslen(j_ci) > _SizeOf(j2f_ci) - 1)
		return 0;

	_tcscpy(j2f_ci, j_ci);
	StringJ2F(j2f_ci);

	//完全相同，不需要插入
	if (!_tcscmp(j2f_ci, f_ci))
		return 0;

	return 1;
}

/**	生成简繁互换词表.
 *	返回：
 *		1：正确处理结束
 *		0：失败
 */
int GenerateJFWordlib(const TCHAR *src_file, const TCHAR *dst_file)
{
	FILE *fr, *fw;
	TCHAR *slot[JF_HASHSLOTS];
	TCHAR j_ci[JF_MAXSTRINGLEN], f_ci[JF_MAXSTRINGLEN];
	int  index[JF_HASHSLOTS];
	int  i, pos, key;
	int  count;

	memset(index, 0, sizeof(index));
	memset(slot, 0, sizeof(slot));

	/* 处理原始简繁词库数据 */
	fr = _tfopen(src_file, TEXT("rb"));
	if (!fr)
	{
		fprintf(stderr, "文件<%s>打开失败", src_file);
		return 0;
	}

	//跳过FFFE
	fseek(fr, 2, SEEK_SET);

	count = 0;
	while(GetStringFromFile(fr, j_ci, JF_MAXSTRINGLEN) &&
		  GetStringFromFile(fr, f_ci, JF_MAXSTRINGLEN))
	{
		if (!NeedInsertJFCi(j_ci, f_ci))
			continue;

		key = GetHashKey(j_ci);
		AddSlotString(&slot[key], j_ci, f_ci);
		count++;
	};
	printf("共有:%d条词汇进入J2F数据文件\n", count);

	fclose(fr);

	/* 处理SLOT的长度(Index) */
	pos = 0;
	for (i = 0; i < JF_HASHSLOTS; i++)
	{
		index[i] = pos;
		if (slot[i])
			pos += (int)_tcslen(slot[i]) * sizeof(TCHAR);
	}

	/* 输出简繁表 */
	fw = _tfopen(dst_file, TEXT("wb"));
	if (!fw)
	{
		fprintf(stderr, "文件<%s>无法创建\n", dst_file);
		return 0;
	}

	if (sizeof(index) != fwrite(index, 1, sizeof(index), fw))
	{
		fprintf(stderr, "文件<%s>写入失败\n", dst_file);
		fclose(fw);
		return 0;
	}

	for (i = 0; i < JF_HASHSLOTS; i++)
	{
		if (!slot[i])
			continue;

		if (_tcslen(slot[i]) * sizeof(TCHAR) != fwrite(slot[i], 1, _tcslen(slot[i]) * sizeof(TCHAR), fw))
		{
			fprintf(stderr, "文件<%s>写入失败\n", dst_file);
			fclose(fw);
			return 0;
		}
	}

	fclose(fw);
	return 1;
}

/**	生成简繁对应词表
 *	返回：
 *		1：正确处理结束
 *		0：失败
 */
int GenerateJFWordList(const TCHAR *src_file, const TCHAR *dst_file)
{
	FILE *fr, *fw;
	TCHAR j_ci[JF_MAXSTRINGLEN], f_ci[JF_MAXSTRINGLEN];
	int  count;

	/* 处理原始简繁词库数据 */
	fr = _tfopen(src_file, TEXT("rb"));
	if (!fr)
	{
		fprintf(stderr, "文件<%s>打开失败", src_file);
		return 0;
	}

	//跳过FFFE
	fseek(fr, 2, SEEK_SET);

	fw = _tfopen(dst_file, TEXT("wt"));
	if (!fw)
	{
		fprintf(stderr, "文件<%s>打开失败", dst_file);
		return 0;
	}

	_setmode(_fileno(fw), _O_U16TEXT);
	_ftprintf(fw, TEXT("%c"), 0xFEFF);

	count = 0;
	while(GetStringFromFile(fr, j_ci, JF_MAXSTRINGLEN) &&
		  GetStringFromFile(fr, f_ci, JF_MAXSTRINGLEN))
	{
		if (!NeedInsertJFCi(j_ci, f_ci))
			continue;

 		_ftprintf(fw, TEXT("%s\t%s\n"), j_ci, f_ci);
		count++;
	};

	printf("共有:%d条词汇进入简繁对应文件\n", count);

	fclose(fr);
	fclose(fw);

	return 1;
}

void UsageExit()
{
	printf("%s", usage);
	exit(-1);
}

int _tmain(int argc, TCHAR* argv[])
{
#if 0
	static UPDATEITEM items[] =
	{
		{"我我我", "wo'wo'wo", 100, 1},
		{"你你你", "ni'ni'ni", 100, 1},
		{"我们", "wo'men", 100, 2},
	};

	return !UpdateWordLibrary("sys.uwl", items, sizeof(items) / sizeof(items[0]));
#endif

	if (argc != 4 && argc != 3 && argc != 2)
		UsageExit();

	if (argc == 2)
	{
		//Test New word table
		if (!_tcscmp(argv[1], TEXT("/T")) || !_tcscmp(argv[1], TEXT("/t")) ||
			!_tcscmp(argv[1], TEXT("/TestNewWord")) || !_tcscmp(argv[1], TEXT("/testnewword")))
			return DoTestNewWordTable();
	}

	if (argc == 3)
	{
		//Upgrade
		if (!_tcscmp(argv[1], TEXT("/U")) || !_tcscmp(argv[1], TEXT("/u")) ||
			!_tcscmp(argv[1], TEXT("/Upgrade")) || !_tcscmp(argv[1], TEXT("/upgrade")))
			return CheckAndUpdateWordLibrary(argv[2]);
	}

	if (argc == 4)
	{
		//Export
		if (!_tcscmp(argv[1], TEXT("/E")) || !_tcscmp(argv[1], TEXT("/e")) ||
			!_tcscmp(argv[1], TEXT("/Export")) || !_tcscmp(argv[1], TEXT("/export")))
			return DoExport(argv[3], argv[2]);

		//Build
		if (!_tcscmp(argv[1], TEXT("/C")) || !_tcscmp(argv[1], TEXT("/c")) ||
			!_tcscmp(argv[1], TEXT("/Create")) || !_tcscmp(argv[1], TEXT("/create")))
			return DoCreate(argv[3], argv[2]);

		//Merge
		if (!_tcscmp(argv[1], TEXT("/I")) || !_tcscmp(argv[1], TEXT("/i")) ||
			!_tcscmp(argv[1], TEXT("/Import")) || !_tcscmp(argv[1], TEXT("/import")))
			return DoImport(argv[3], argv[2]);

		if (!_tcscmp(argv[1], TEXT("/G")) || !_tcscmp(argv[1], TEXT("/g")) ||
			!_tcscmp(argv[1], TEXT("/Genj2f")) || !_tcscmp(argv[1], TEXT("genj2f")))
			return !GenerateJFWordlib(argv[3], argv[2]);

		if (!_tcscmp(argv[1], TEXT("/GT")) || !_tcscmp(argv[1], TEXT("/gt")))
			return !GenerateJFWordList(argv[3], argv[2]);		
	}

	UsageExit();
	return -1;
}