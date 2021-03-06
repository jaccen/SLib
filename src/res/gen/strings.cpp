#include "strings.h"

namespace slib
{
	namespace string
	{

		SLIB_DEFINE_STRING_RESOURCE_BEGIN(cancel, "Cancel")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(ko, "\xec\xb7\xa8\xec\x86\x8c")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(zh, "\xe5\x8f\x96\xe6\xb6\x88")
		SLIB_DEFINE_STRING_RESOURCE_END

		SLIB_DEFINE_STRING_RESOURCE_BEGIN(no, "No")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(ko, "\xec\x95\x84\xeb\x8b\x88")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(zh, "\xe5\x90\xa6")
		SLIB_DEFINE_STRING_RESOURCE_END

		SLIB_DEFINE_STRING_RESOURCE_BEGIN(ok, "OK")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(ko, "\xed\x99\x95\xec\x9d\xb8")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(zh, "\xe7\xa1\xae\xe5\xae\x9a")
		SLIB_DEFINE_STRING_RESOURCE_END

		SLIB_DEFINE_STRING_RESOURCE_BEGIN(today, "Today")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(ko, "\xec\x98\xa4\xeb\x8a\x98")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(zh, "\xe4\xbb\x8a\xe5\xa4\xa9")
		SLIB_DEFINE_STRING_RESOURCE_END

		SLIB_DEFINE_STRING_RESOURCE_BEGIN(yes, "Yes")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(ko, "\xec\x98\x88")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(zh, "\xe6\x98\xaf")
		SLIB_DEFINE_STRING_RESOURCE_END

		SLIB_DEFINE_STRING_RESOURCE_BEGIN(yesterday, "Yesterday")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(ko, "\xec\x96\xb4\xec\xa0\x9c")
			SLIB_DEFINE_STRING_RESOURCE_VALUE(zh, "\xe6\x98\xa8\xe5\xa4\xa9")
		SLIB_DEFINE_STRING_RESOURCE_END

		SLIB_DEFINE_STRING_RESOURCE_MAP_BEGIN
			SLIB_DEFINE_STRING_RESOURCE_MAP_ITEM(cancel)
			SLIB_DEFINE_STRING_RESOURCE_MAP_ITEM(no)
			SLIB_DEFINE_STRING_RESOURCE_MAP_ITEM(ok)
			SLIB_DEFINE_STRING_RESOURCE_MAP_ITEM(today)
			SLIB_DEFINE_STRING_RESOURCE_MAP_ITEM(yes)
			SLIB_DEFINE_STRING_RESOURCE_MAP_ITEM(yesterday)
		SLIB_DEFINE_STRING_RESOURCE_MAP_END

	}
}
