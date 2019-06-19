
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "gmock/gmock.h"
#	include "common_cppunit_headers.h"
#	include "expose_stub_classes.h"
#	include "expose_userdata_function_params.h"
#	include "expose_integral_function_params.h"
#	include "expose_false_integral_function_params.h"
#	include <stdexcept>
#	include <csetjmp>

#if OOLUA_DEBUG_CHECKS == 1
	/*This seems to be the only way of using the preprocesser and detecting luajit,
	 even when the path is pointing to luajit header directory.
	 If we include the luajit header directly it will fail to compile for Rio Lua
	 */
//#	include "lua/lua.hpp"
#endif


#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4702)//unreachable code
#endif
namespace
{
	jmp_buf mark;
	int OOLua_panic(lua_State* /*vm*/) //NOLINT(readability/casting)
	{
		longjmp(mark, 1);
		return 0;
	}
	enum  SomeEnum {someEnumValue};
} //namespace

#ifdef _MSC_VER
#	pragma warning(pop)
#endif


#if OOLUA_USE_EXCEPTIONS ==  1
void cFunctionNoReturn_throwsStdRuntimeError()
{
	throw std::runtime_error("This must not escape");
}
OOLUA_CFUNC(cFunctionNoReturn_throwsStdRuntimeError, l_cFunctionNoReturn_throwsStdRuntimeError)

int cFunctionWithReturn_throwsStdRuntimeError()
{
	throw std::runtime_error("This must not escapee");
	return 1;
}
OOLUA_CFUNC(cFunctionWithReturn_throwsStdRuntimeError, l_cFunctionWithReturn_throwsStdRuntimeError)


struct ExceptionMock
{
	void throwsStdRuntimeError()
	{
		throw std::runtime_error("member function throwing an error");
	}
};



OOLUA_PROXY(ExceptionMock)
	OOLUA_TAGS(
		No_public_constructors
		, No_public_destructor
	)
	OOLUA_MEM_FUNC(void, throwsStdRuntimeError)
OOLUA_PROXY_END

OOLUA_EXPORT_FUNCTIONS(ExceptionMock, throwsStdRuntimeError)
OOLUA_EXPORT_FUNCTIONS_CONST(ExceptionMock)
#endif


struct NewProblemBase
{
	virtual ~NewProblemBase(){}
};
struct NewProblemAbstract : NewProblemBase
{
	virtual ~NewProblemAbstract(){}
	virtual void foo() = 0;
};
struct NewProblemDerived : NewProblemAbstract
{
	virtual ~NewProblemDerived(){}
	void foo(){}
};
OOLUA_PROXY(NewProblemBase)
OOLUA_PROXY_END
OOLUA_EXPORT_NO_FUNCTIONS(NewProblemBase)

OOLUA_PROXY(NewProblemAbstract, NewProblemBase)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(foo)
OOLUA_PROXY_END
OOLUA_EXPORT_FUNCTIONS(NewProblemAbstract, foo)
OOLUA_EXPORT_FUNCTIONS_CONST(NewProblemAbstract)

OOLUA_PROXY(NewProblemDerived, NewProblemAbstract)
OOLUA_PROXY_END
OOLUA_EXPORT_NO_FUNCTIONS(NewProblemDerived)


void cfunctionTakesStub(Stub1*){}//NOLINT(readability/function)
int l_cfunctionTakesStub(lua_State* vm)
{
	OOLUA_C_FUNCTION(void, cfunctionTakesStub, OOLUA::cpp_in_p<Stub1*>)
}

void * dummy_allocator(void* /*ud*/, void* /*ptr*/, size_t /*osize*/, size_t /*nsize*/) //NOLINT(readability/casting)
{ return 0; }


class Error_test : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Error_test);
		CPPUNIT_TEST(userDataCheck_constUserdataOnTopOfStackWhichOoluaDidCreate_resultIsTrue);
		CPPUNIT_TEST(userDataCheck_UserdataOnTopOfStackWhichOoluaDidCreate_resultIsTrue);
		CPPUNIT_TEST(userDataCheck_userdataOnTopOfStackWhichARelatedThreadCreated_resultIsTrue);
		CPPUNIT_TEST(userDataCheck_UserdataOnTopOfStackWhichOoluaDidNotCreate_stackIsTheSameSizeAfterCheck);

		CPPUNIT_TEST(registerClass_checkStackSize_stackIsEmpty);
		CPPUNIT_TEST(scriptConstructor_checkStackSize_stackIsEmpty);
		CPPUNIT_TEST(lua_Lopenlibs_checkStackSizeAfterCall_stackIsEmpty);

#if OOLUA_RUNTIME_CHECKS_ENABLED == 1
#	if OOLUA_CHECK_EVERY_USERDATA_IS_CREATED_BY_OOLUA == 1
		CPPUNIT_TEST(userDataCheck_UserdataOnTopOfStackWhichOoluaDidNotCreate_resultIsFalse);
		CPPUNIT_TEST(userDataCheck_lightUserDataWithNoMetaTable_resultIsFalse);
#		if OOLUA_USERDATA_OPTIMISATION == 1
			CPPUNIT_TEST(userData_craftUserDataWhichCorrectSizeYetNotCookie_isUserdataReturnsFalse);
#		endif
#	endif
#	if OOLUA_USE_EXCEPTIONS == 1
		CPPUNIT_TEST(memberFunctionCall_luaSelfCallOnType_throwsOoluaRunTimeError);
#	elif OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(memberFunctionCall_luaSelfCallOnType_callRetunsFalse);
#	endif

#endif

#if OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(lastError_noError_lastErrorStringIsEmpty);

		CPPUNIT_TEST(lastError_callUnknownFunction_lastErrorStringIsNotEmpty);
		CPPUNIT_TEST(call_callUnknownFunction_callReturnsFalse);
		CPPUNIT_TEST(lastError_callUnknownFunction_stackIsEmpty);
		CPPUNIT_TEST(errorReset_callUnknownFunctionThenReset_lastErrorStringIsEmpty);

		CPPUNIT_TEST(pull_UnrelatedClassType_pullReturnsFalse);
		CPPUNIT_TEST(pull_UnrelatedClassType_ptrIsNull);
		CPPUNIT_TEST(pull_UnrelatedClassType_lastErrorStringIsNotEmpty);

		CPPUNIT_TEST(pull_FunctionTriesToAcquirePtrWhenWrongClassTypeIsOnTheStack_callReturnsFalse);
		CPPUNIT_TEST(pull_FunctionTriesToAcquirePtrWhenWrongClassTypeIsOnTheStack_lastErrorStringIsNotEmpty);

		CPPUNIT_TEST(pull_classWhenintIsOnStack_lastErrorStringIsNotEmpty);
		CPPUNIT_TEST(pull_classWhenintIsOnStack_pullReturnsFalse);
		CPPUNIT_TEST(pull_intWhenClassIsOnStack_pullReturnsFalse);
		CPPUNIT_TEST(pull_enumWhenStringIsOnStack_callReturnsFalse);
		CPPUNIT_TEST(pull_memberFunctionPullsClassWhenintIsOnStack_callReturnsFalse);
		CPPUNIT_TEST(pull_CFunctionFromStackTopIsNotFunc_callReturnsFalse);
		CPPUNIT_TEST(pull_CFunctionFromStackTopIsNotFunc_errorStringIsNotEmpty);
		CPPUNIT_TEST(loadFile_fileDoesNotExist_returnsFalse);
		CPPUNIT_TEST(runFile_fileDoesNotExist_returnsFalse);
		CPPUNIT_TEST(new_onAbstractClass_runChunkReturnsFalse);

		CPPUNIT_TEST(luaFunctionCall_luaPassesBooleanToFunctionWantingInt_callReturnsFalse);
		CPPUNIT_TEST(luaFunctionCall_luaPassesBooleanToFunctionWantingInt_lastErrorHasAnEntry);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesTableYetPassedInt_callReturnsFalse);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesFloatYetPassedTable_callReturnsFalse);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesDoubleYetPassedTable_callReturnsFalse);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesLuaCFunctionYetPassedTable_callReturnsFalse);
#endif

#if OOLUA_USE_EXCEPTIONS == 1

		CPPUNIT_TEST(pull_ptrToconstUserDataTypeWhenStackIsNoneOoluaUserData_throwsTypeError);

		CPPUNIT_TEST(pull_UnrelatedClassType_throwsTypeError);

		CPPUNIT_TEST(callUnknownFunction_fromCpp_throwsOoluaRuntimeError);

		CPPUNIT_TEST(runChunk_chunkHasSyntaxError_throwSyntaxError);

		CPPUNIT_TEST(pullUnregisteredClass_fromEmptyStackInCpp_throwTypeError);
		CPPUNIT_TEST(pullUnregisteredClass_fromStackContainingAnIntInCpp_throwTypeError);

		CPPUNIT_TEST(pull_classWhenintIsOnStack_throwsTypeError);

		CPPUNIT_TEST(pull_memberFunctionPullsClassWhenintIsOnStack_throwsOoluaRuntimeError);

		CPPUNIT_TEST(pull_intWhenClassIsOnStack_throwsTypeError);

		CPPUNIT_TEST(pull_boolFromEmptyStack_throwTypeError);
		CPPUNIT_TEST(pull_pushAnIntThenPullBool_throwTypeError);
		CPPUNIT_TEST(pull_pushBoolThenPullInt_throwTypeError);

		CPPUNIT_TEST(pull_pushIntThenPullFloat_noException);

		CPPUNIT_TEST(pull_enumWhenStringIsOnStack_throwTypeError);

		CPPUNIT_TEST(pull_cppAcquirePtrWhenIntOnStack_throwsTypeError);
		CPPUNIT_TEST(pull_FunctionTriesToAcquirePtrWhenWrongClassTypeIsOnTheStack_throwsRuntimeError);

		CPPUNIT_TEST(pull_CFunctionFromStackTopIsNotFunc_throwsRunTimeError);

		CPPUNIT_TEST(exceptionSafe_memberFunctionThrowsStdRuntimeError_callThrowsOoluaRuntimeError);
		CPPUNIT_TEST(call_afterAnExceptionTheStackIsEmpty_stackCountEqualsZero);

		CPPUNIT_TEST(loadFile_fileDoesNotExist_callThrowsOoluaFileError);
		CPPUNIT_TEST(runFile_fileDoesNotExist_callThrowsOoluaFileError);

		CPPUNIT_TEST(new_onAbstractClass_runChunkThrowsStdRuntimeError);

		CPPUNIT_TEST(luaFunctionCall_luaPassesBooleanToFunctionWantingInt_throwsRuntimeError);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesTableYetPassedInt_throwsRuntimeError);

		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesFloatYetPassedTable_throwsRuntimeError);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesDoubleYetPassedTable_throwsRuntimeError);
		CPPUNIT_TEST(memberFunctionCall_memberFunctionWhichTakesLuaCFunctionYetPassedTable_throwsRuntimeError);

		CPPUNIT_TEST(cFunctionNoReturn_throwsStdRuntimeError_exceptionDoesNotEscapePcall);
		CPPUNIT_TEST(cFunctionWithReturn_throwsStdRuntimeError_exceptionDoesNotEscapePcall);

		CPPUNIT_TEST(exception_assignStringOfLengthOnePlusNull_bufferLenIsOne);
		CPPUNIT_TEST(exception_assignStringOfLengthOnePlusNull_bufferIndexOneIsNullToken);
		CPPUNIT_TEST(exceptionBufferIsTrimmedCorrectly_assignStringOfOneGreaterThanExceptionBufferCanContain_memberLenEqualsBufferSizeMinusTwo);
		CPPUNIT_TEST(exceptionBufferIsTrimmedCorrectly_assignStringOfOneGreaterThanExceptionBufferCanCotain_bufferMaxSizeMinusOneIsNullToken);
		CPPUNIT_TEST(exceptionPopsStack_pushStringToStackAndPassVmAndAskToPopTheStack_stackSizeIsZero);
		CPPUNIT_TEST(exceptionDoesntPopStack_pushStringToStackAndPassVm_stackSizeIsOne);
#endif



/* ====================== LuaJIT2 protected tests ===========================*/
#if OOLUA_DEBUG_CHECKS == 1
		CPPUNIT_TEST(push_unregisteredClass_callsLuaPanic);
#endif
/* ====================== LuaJIT2 protected tests ===========================*/

		CPPUNIT_TEST(canXmove_vm0IsNULL_returnsFalse);
		CPPUNIT_TEST(canXmove_vm1IsNULL_returnsFalse);
		CPPUNIT_TEST(canXmove_samePointers_returnsFalse);
		CPPUNIT_TEST(canXmove_ParentAndChild_returnsTrue);
		CPPUNIT_TEST(canXmove_unrelatedStates_returnsFalse);
		CPPUNIT_TEST(canXmove_ValidParentChildBothWithAStackCountOfOne_bothHaveStackCountIsOneAfterCall);

	CPPUNIT_TEST_SUITE_END();
	OOLUA::Script * m_lua;
public:
	Error_test()
		: m_lua(0)
	{}
	LVD_NOCOPY(Error_test)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void userDataCheck_runFunction()
	{
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->register_class<Stub1>();
		m_lua->call("foo");
	}

	//if the type is a userdata it will always returns true if runtine errors turned off
	void userDataCheck_UserdataOnTopOfStackWhichOoluaDidCreate_resultIsTrue()
	{
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->register_class<Stub1>();
		m_lua->call("foo");
		OOLUA::INTERNAL::Lua_ud* dontCare;
		bool result = OOLUA::INTERNAL::index_is_userdata(*m_lua, -1, dontCare);
		CPPUNIT_ASSERT_EQUAL(true, result);
	}

	void userDataCheck_userdataOnTopOfStackWhichARelatedThreadCreated_resultIsTrue()
	{
		m_lua->run_chunk("foo = function() "
						 "local c = coroutine.create(function() return Stub1.new() end) "
						 "local err, result = coroutine.resume(c) "
						 "if err == false then error(result) end "
						 "return result "
						 "end");
		m_lua->register_class<Stub1>();
		CPPUNIT_ASSERT(m_lua->call("foo"));
		OOLUA::INTERNAL::Lua_ud* dontCare;
		bool result = OOLUA::INTERNAL::index_is_userdata(*m_lua, -1, dontCare);
		CPPUNIT_ASSERT_EQUAL(true, result);
	}

	void userDataCheck_constUserdataOnTopOfStackWhichOoluaDidCreate_resultIsTrue()
	{
		m_lua->run_chunk("foo = function(obj) "
						 "return obj "
						 "end");
		m_lua->register_class<Stub1>();
		Stub1 s;
		Stub1 const * struct_ptr = &s;
		m_lua->call("foo", struct_ptr);
		OOLUA::INTERNAL::Lua_ud* dontCare;
		bool result = OOLUA::INTERNAL::index_is_userdata(*m_lua, -1, dontCare);
		CPPUNIT_ASSERT_EQUAL(true, result);
	}

	//if runtime checks turned off it will never effect the stack size
	void userDataCheck_UserdataOnTopOfStackWhichOoluaDidNotCreate_stackIsTheSameSizeAfterCheck()
	{
		lua_newuserdata(*m_lua, sizeof(int));
		int before = lua_gettop(*m_lua);
		OOLUA::INTERNAL::Lua_ud* dontCare;
		OOLUA::INTERNAL::index_is_userdata(*m_lua, -1, dontCare);
		int after = lua_gettop(*m_lua);
		CPPUNIT_ASSERT_EQUAL(before, after);
	}

	void userData_craftUserDataWhichCorrectSizeYetNotCookie_isUserdataReturnsFalse()
	{
		OOLUA::INTERNAL::Lua_ud* ud = (OOLUA::INTERNAL::Lua_ud*)lua_newuserdata(*m_lua, sizeof(OOLUA::INTERNAL::Lua_ud) );
		memset(ud, 0, sizeof(*ud));
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::index_is_userdata(*m_lua, 1, ud));
	}

	void scriptConstructor_checkStackSize_stackIsEmpty()
	{
		OOLUA::Script s;
		CPPUNIT_ASSERT_EQUAL(0, s.stack_count());
	}

	void registerClass_checkStackSize_stackIsEmpty()
	{
		OOLUA::Script s;
		s.register_class<Stub1>();
		CPPUNIT_ASSERT_EQUAL(0, s.stack_count());
	}

	void lua_Lopenlibs_checkStackSizeAfterCall_stackIsEmpty()
	{
		lua_State* vm = luaL_newstate();
		luaL_openlibs(vm);
		CPPUNIT_ASSERT_EQUAL(0, lua_gettop(vm));
	}


#if OOLUA_RUNTIME_CHECKS_ENABLED == 1

#	if OOLUA_USE_EXCEPTIONS == 1
	void memberFunctionCall_luaSelfCallOnType_throwsOoluaRunTimeError()
	{
		m_lua->register_class<IntegerFunctionInTraits>();
		CPPUNIT_ASSERT_THROW((m_lua->run_chunk("IntegerFunctionInTraits:value(1)")), OOLUA::Runtime_error);
	}
#	elif OOLUA_STORE_LAST_ERROR == 1
	void memberFunctionCall_luaSelfCallOnType_callRetunsFalse()
	{
		m_lua->register_class<IntegerFunctionInTraits>();
		bool result = m_lua->run_chunk("IntegerFunctionInTraits:value(1)");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}
#	endif

	void userDataCheck_lightUserDataWithNoMetaTable_resultIsFalse()
	{
		lua_pushlightuserdata(*m_lua, this);
		OOLUA::INTERNAL::Lua_ud* dontCare;
		bool result = OOLUA::INTERNAL::index_is_userdata(*m_lua, -1, dontCare);
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void userDataCheck_UserdataOnTopOfStackWhichOoluaDidNotCreate_resultIsFalse()
	{
		lua_newuserdata(*m_lua, sizeof(int));
		OOLUA::INTERNAL::Lua_ud* dontCare;
		bool result = OOLUA::INTERNAL::index_is_userdata(*m_lua, -1, dontCare);
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

#endif


#if OOLUA_STORE_LAST_ERROR == 1

	void pull_ptrToconstUserDataTypeWhenStackIsNoneOoluaUserData_pullResultIsFalse()
	{
		lua_newuserdata(*m_lua, sizeof(int));
		Stub1 const*  cpp_type =  0;
		bool result = OOLUA::pull(*m_lua, cpp_type);
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void pull_enumWhenStringIsOnStack_callReturnsFalse()
	{
		m_lua->run_chunk("foo = function()return 'DontCareAboutStringValue' end");
		m_lua->call("foo");

		SomeEnum enum_value;
		bool result = OOLUA::pull(*m_lua, enum_value);
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void pull_memberFunctionPullsClassWhenintIsOnStack_callReturnsFalse()
	{
		m_lua->run_chunk("foo = function(obj)"
						 " obj:ptr(1) "
						 "end");
		m_lua->register_class<InParamUserData>();
		m_lua->register_class<HasIntMember>();
		::testing::NiceMock<InParamUserDataMock> object;
		bool result = m_lua->call("foo", static_cast<InParamUserData*>(&object));
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void lastError_noError_lastErrorStringIsEmpty()
	{
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::get_last_error(*m_lua).empty() );
	}

	void lastError_callUnknownFunction_lastErrorStringIsNotEmpty()
	{
		m_lua->call("foo");
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(*m_lua).empty() );
	}

	void call_callUnknownFunction_callReturnsFalse()
	{
		m_lua->run_chunk("foo = function() "
						 "bar() "
						 "end");
		m_lua->call("foo");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call("foo"));
	}

	void lastError_callUnknownFunction_stackIsEmpty()
	{
		m_lua->run_chunk("foo = function() "
						 "bar() "
						 "end");
		m_lua->call("foo");
		CPPUNIT_ASSERT_EQUAL(0, m_lua->stack_count() );
	}

	void errorReset_callUnknownFunctionThenReset_lastErrorStringIsEmpty()
	{
		m_lua->call("foo");
		OOLUA::reset_error_value(*m_lua);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::get_last_error(*m_lua).empty() );
	}


	void pull_UnrelatedClassType_pullReturnsFalse()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<InvalidStub>();
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->call("foo");
		InvalidStub* ptr;
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::pull(*m_lua, ptr));
	}

	void pull_UnrelatedClassType_ptrIsNull()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<InvalidStub>();
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->call("foo");
		InvalidStub* ptr;
		OOLUA::pull(*m_lua, ptr);
		CPPUNIT_ASSERT_EQUAL(static_cast<InvalidStub*>(0), ptr);
	}

	void pull_UnrelatedClassType_lastErrorStringIsNotEmpty()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<InvalidStub>();
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->call("foo");
		InvalidStub* ptr;
		OOLUA::pull(*m_lua, ptr);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(*m_lua).empty() );
	}

	void pull_FunctionTriesToAcquirePtrWhenWrongClassTypeIsOnTheStack_callReturnsFalse()
	{
		m_lua->register_class<Stub2>();
		Stub2 f;
		m_lua->push(l_cfunctionTakesStub);
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, &f));
	}

	void pull_FunctionTriesToAcquirePtrWhenWrongClassTypeIsOnTheStack_lastErrorStringIsNotEmpty()
	{
		m_lua->register_class<Stub2>();
		Stub2 f;
		m_lua->push(l_cfunctionTakesStub);
		m_lua->call(1, &f);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(*m_lua).empty() );
	}

	void pull_classWhenintIsOnStack_lastErrorStringIsNotEmpty()
	{
		m_lua->register_class<Stub1>();
		m_lua->run_chunk("foo = function() "
						 "return 1 "
						 "end");
		m_lua->call("foo");
		Stub1* result;
		OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(*m_lua).empty() );
	}

	void pull_classWhenintIsOnStack_pullReturnsFalse()
	{
		m_lua->register_class<Stub1>();
		m_lua->run_chunk("foo = function() "
						 "return 1 "
						 "end");
		m_lua->call("foo");
		Stub1* result;
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::pull(*m_lua, result));
	}

	void pull_intWhenClassIsOnStack_pullReturnsFalse()
	{
		m_lua->register_class<Stub1>();
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->call("foo");
		int result;
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::pull(*m_lua, result));
	}

	void pull_CFunctionFromStackTopIsNotFunc_callReturnsFalse()
	{
		OOLUA::Lua_func_ref func;
		bool result = OOLUA::pull(*m_lua, func);
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void pull_CFunctionFromStackTopIsNotFunc_errorStringIsNotEmpty()
	{
		OOLUA::Lua_func_ref func;
		OOLUA::pull(*m_lua, func);
		std::string error_str = OOLUA::get_last_error(*m_lua);
		CPPUNIT_ASSERT_EQUAL(false, error_str.empty());
	}

	void loadFile_fileDoesNotExist_returnsFalse()
	{
		CPPUNIT_ASSERT_EQUAL(false, m_lua->load_file("does_not_exist"));
	}

	void runFile_fileDoesNotExist_returnsFalse()
	{
		CPPUNIT_ASSERT_EQUAL(false, m_lua->run_file("does_not_exist"));
	}

	void new_onAbstractClass_runChunkReturnsFalse()
	{
		OOLUA::register_class<NewProblemDerived>(*m_lua);
		CPPUNIT_ASSERT_EQUAL(false, m_lua->run_chunk("return NewProblemAbstract.new()"));
	}

	void luaFunctionCall_luaPassesBooleanToFunctionWantingInt_callReturnsFalse()
	{
		m_lua->register_class<IntegerFunctionInTraits>();
		m_lua->run_chunk("foo = function(obj) "
						 "obj:value(true) "
						 "end");
		::testing::NiceMock<IntegerFunctionInTraitsMock> instance;
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call("foo", static_cast<IntegerFunctionInTraits*>(&instance)));
	}

	void luaFunctionCall_luaPassesBooleanToFunctionWantingInt_lastErrorHasAnEntry()
	{
		m_lua->register_class<IntegerFunctionInTraits>();
		m_lua->run_chunk("foo = function(obj) "
						 "obj:value(true) "
						 "end");
		::testing::NiceMock<IntegerFunctionInTraitsMock> instance;
		m_lua->call("foo", static_cast<IntegerFunctionInTraits*>(&instance));
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(*m_lua).empty());
	}

	void memberFunctionCall_memberFunctionWhichTakesTableYetPassedInt_callReturnsFalse()
	{
		::testing::NiceMock<TableObjectParamMock> mock;
		TableObjectParam* object(&mock);

		m_lua->register_class<TableObjectParam>();
		m_lua->run_chunk("func = function(obj) "
							"return obj:value(1) "
						 "end");

		CPPUNIT_ASSERT_EQUAL(false, m_lua->call("func", object));
	}

	void memberFunctionCall_memberFunctionWhichTakesFloatYetPassedTable_callReturnsFalse()
	{
		::testing::NiceMock<FloatFunctionInTraitsMock> stub;
		FloatFunctionInTraits* object = &stub;
		m_lua->register_class<FloatFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:value{} end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, object));
	}

	void memberFunctionCall_memberFunctionWhichTakesDoubleYetPassedTable_callReturnsFalse()
	{
		::testing::NiceMock<DoubleFunctionInTraitsMock> stub;
		DoubleFunctionInTraits* object = &stub;
		m_lua->register_class<DoubleFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:value{} end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, object));
	}

	void memberFunctionCall_memberFunctionWhichTakesLuaCFunctionYetPassedTable_callReturnsFalse()
	{
		::testing::NiceMock<CFunctionInTraitsMock> stub;
		CFunctionInTraits* object = &stub;
		m_lua->register_class<CFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:value{} end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, object));
	}
#endif

#if OOLUA_USE_EXCEPTIONS == 1

	void pull_ptrToconstUserDataTypeWhenStackIsNoneOoluaUserData_throwsTypeError()
	{
		lua_newuserdata(*m_lua, sizeof(int));
		Stub1 const*  cpp_type =  0;
		CPPUNIT_ASSERT_THROW((OOLUA::pull(*m_lua, cpp_type)), OOLUA::Type_error);
	}

	void pull_cppAcquirePtrWhenIntOnStack_throwsTypeError()
	{
		m_lua->run_chunk("foo = function() return 1 end");
		m_lua->call("foo");
		OOLUA::cpp_acquire_ptr<Stub1*> cpp_type;
		CPPUNIT_ASSERT_THROW((OOLUA::pull(*m_lua, cpp_type)), OOLUA::Type_error);
	}

	void pull_FunctionTriesToAcquirePtrWhenWrongClassTypeIsOnTheStack_throwsRuntimeError()
	{
		m_lua->register_class<Stub2>();
		Stub2 f;
		m_lua->push(l_cfunctionTakesStub);
		CPPUNIT_ASSERT_THROW(m_lua->call(1, &f), OOLUA::Runtime_error);
	}

	void pull_enumWhenStringIsOnStack_throwTypeError()
	{
		m_lua->run_chunk("foo = function()return 'DontCareAboutStringValue' end");
		m_lua->call("foo");

		SomeEnum enum_value;
		CPPUNIT_ASSERT_THROW((OOLUA::pull(*m_lua, enum_value)), OOLUA::Type_error);
	}

	void pull_memberFunctionPullsClassWhenintIsOnStack_throwsOoluaRuntimeError()
	{
		m_lua->run_chunk("foo = function(obj)"
						 " obj:ptr(1) "
						 "end");
		m_lua->register_class<InParamUserData>();
		m_lua->register_class<HasIntMember>();
		::testing::NiceMock<InParamUserDataMock> object;
		CPPUNIT_ASSERT_THROW(m_lua->call("foo", static_cast<InParamUserData*>(&object)), OOLUA::Runtime_error);
	}

	void pull_UnrelatedClassType_throwsTypeError()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<InvalidStub>();
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->call("foo");
		InvalidStub* ptr;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, ptr), OOLUA::Type_error);
	}

	void pull_classWhenintIsOnStack_throwsTypeError()
	{
		m_lua->register_class<Stub1>();
		m_lua->run_chunk("foo = function() "
						 "return 1 "
						 "end");
		m_lua->call("foo");
		Stub1* result;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, result), OOLUA::Type_error);
	}

	void pull_intWhenClassIsOnStack_throwsTypeError()
	{
		m_lua->register_class<Stub1>();
		m_lua->run_chunk("foo = function() "
						 "return Stub1.new() "
						 "end");
		m_lua->call("foo");
		int result;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, result), OOLUA::Type_error);
	}

	void pull_CFunctionFromStackTopIsNotFunc_throwsRunTimeError()
	{
		OOLUA::Lua_func_ref func;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, func), OOLUA::Runtime_error);
	}

	void callUnknownFunction_fromCpp_throwsOoluaRuntimeError()
	{
		CPPUNIT_ASSERT_THROW(m_lua->call("InvalidFunctionName"), OOLUA::Runtime_error);
	}

	void runChunk_chunkHasSyntaxError_throwSyntaxError()
	{
		CPPUNIT_ASSERT_THROW(m_lua->run_chunk("missingEnd = function() "), OOLUA::Syntax_error);
	}

	void pullUnregisteredClass_fromEmptyStackInCpp_throwTypeError()
	{
		Stub1* fp(0);
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, fp), OOLUA::Type_error);
	}

	void pullUnregisteredClass_fromStackContainingAnIntInCpp_throwTypeError()
	{
		Stub1* fp(0);
		int i =1;
		OOLUA::push(*m_lua, i);
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, fp), OOLUA::Type_error);
	}

	void pull_boolFromEmptyStack_throwTypeError()
	{
		bool stub;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, stub), OOLUA::Type_error);
	}

	void pull_pushIntThenPullFloat_noException()
	{
		int dontCare(0);
		float pullInto(0.f);
		OOLUA::push(*m_lua, dontCare);
		OOLUA::pull(*m_lua, pullInto);
	}

	void pull_pushAnIntThenPullBool_throwTypeError()
	{
		int dontCare(0);
		OOLUA::push(*m_lua, dontCare);
		bool pullInto;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, pullInto), OOLUA::Type_error);
	}

	void pull_pushBoolThenPullInt_throwTypeError()
	{
		//bool is a distinct type from int and float
		bool dontCare(false);
		int pullInto(0);
		OOLUA::push(*m_lua, dontCare);
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, pullInto), OOLUA::Type_error);
	}

	void registerExceptionMockAndRunChunkCalledFoo()
	{
		m_lua->register_class<ExceptionMock>();
		m_lua->run_chunk("foo = function(obj) "
						 "obj:throwsStdRuntimeError() "
						 "end");
	}
	void exceptionSafe_memberFunctionThrowsStdRuntimeError_callThrowsOoluaRuntimeError()
	{
		ExceptionMock m;
		registerExceptionMockAndRunChunkCalledFoo();
		CPPUNIT_ASSERT_THROW(m_lua->call("foo", &m), OOLUA::Runtime_error);
	}

	void call_afterAnExceptionTheStackIsEmpty_stackCountEqualsZero()
	{
		ExceptionMock m;
		registerExceptionMockAndRunChunkCalledFoo();

		CPPUNIT_ASSERT_THROW(m_lua->call("foo", &m), OOLUA::Runtime_error);
		CPPUNIT_ASSERT_EQUAL(0, m_lua->stack_count());
	}

	void loadFile_fileDoesNotExist_callThrowsOoluaFileError()
	{
		CPPUNIT_ASSERT_THROW(m_lua->load_file("does_not_exist"), OOLUA::File_error);
	}

	void runFile_fileDoesNotExist_callThrowsOoluaFileError()
	{
		CPPUNIT_ASSERT_THROW(m_lua->run_file("does_not_exist"), OOLUA::File_error);
	}

	void new_onAbstractClass_runChunkThrowsStdRuntimeError()
	{
		OOLUA::register_class<NewProblemDerived>(*m_lua);
		CPPUNIT_ASSERT_THROW(m_lua->run_chunk("return NewProblemAbstract.new()"), OOLUA::Runtime_error);
	}

	void luaFunctionCall_luaPassesBooleanToFunctionWantingInt_throwsRuntimeError()
	{
		m_lua->register_class<IntegerFunctionInTraits>();
		m_lua->run_chunk("foo = function(obj) "
						 "obj:value(true) "
						 "end");
		::testing::NiceMock<IntegerFunctionInTraitsMock> instance;
		CPPUNIT_ASSERT_THROW(m_lua->call("foo", static_cast<IntegerFunctionInTraits*>(&instance)), OOLUA::Runtime_error);
	}

	void memberFunctionCall_memberFunctionWhichTakesTableYetPassedInt_throwsRuntimeError()
	{
		::testing::NiceMock<TableObjectParamMock> mock;
		TableObjectParam* object(&mock);

		m_lua->register_class<TableObjectParam>();
		m_lua->run_chunk("func = function(obj) "
							"return obj:value(1) "
						 "end");

		CPPUNIT_ASSERT_THROW(m_lua->call("func", object), OOLUA::Runtime_error);
	}

	void string_ptrParamPassedInt_runtimeError()
	{
		CharFunctionInTraitsMock mock;
		CharFunctionInTraits* object(&mock);
		m_lua->register_class<CharFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:ptr(1) end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, object), OOLUA::Runtime_error);
	}

	void memberFunctionCall_memberFunctionWhichTakesFloatYetPassedTable_throwsRuntimeError()
	{
		::testing::NiceMock<FloatFunctionInTraitsMock> stub;
		FloatFunctionInTraits* object = &stub;
		m_lua->register_class<FloatFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:value{} end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, object), OOLUA::Runtime_error);
	}

	void memberFunctionCall_memberFunctionWhichTakesDoubleYetPassedTable_throwsRuntimeError()
	{
		::testing::NiceMock<DoubleFunctionInTraitsMock> stub;
		DoubleFunctionInTraits* object = &stub;
		m_lua->register_class<DoubleFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:value{} end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, object), OOLUA::Runtime_error);
	}

	void memberFunctionCall_memberFunctionWhichTakesLuaCFunctionYetPassedTable_throwsRuntimeError()
	{
		::testing::NiceMock<CFunctionInTraitsMock> stub;
		CFunctionInTraits* object = &stub;
		m_lua->register_class<CFunctionInTraits>();
		m_lua->run_chunk("return function(object) object:value{} end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, object), OOLUA::Runtime_error);
	}

	void cFunctionNoReturn_throwsStdRuntimeError_exceptionDoesNotEscapePcall()
	{
		OOLUA::set_global(*m_lua, "throwsException", l_cFunctionNoReturn_throwsStdRuntimeError);
		CPPUNIT_ASSERT_NO_THROW(m_lua->run_chunk("pcall(throwsException)"));
	}

	void cFunctionWithReturn_throwsStdRuntimeError_exceptionDoesNotEscapePcall()
	{
		OOLUA::set_global(*m_lua, "throwsException", l_cFunctionWithReturn_throwsStdRuntimeError);
		CPPUNIT_ASSERT_NO_THROW(m_lua->run_chunk("pcall(throwsException)"));
	}

	void exception_assignStringOfLengthOnePlusNull_bufferLenIsOne()
	{
		OOLUA::Exception e("f");
		size_t expected_value(1);
		CPPUNIT_ASSERT_EQUAL(expected_value, e.m_len);
	}

	void exception_assignStringOfLengthOnePlusNull_bufferIndexOneIsNullToken()
	{
		//Logic here to make it fail.
		//This may only fail in release as the debug runtime is nice and may intialise data Yeah!
		//We can not know what the buffer data will contain for sure.
		//So we assign a buffer longer than we want, correct the size member
		//and then copy the data
		OOLUA::Exception e("DontCare");
		e.m_len = 1;
		OOLUA::Exception corrected(e);
		char expected_value('\0');
		CPPUNIT_ASSERT_EQUAL(expected_value, corrected.m_buffer[1]);
	}

	void exceptionBufferIsTrimmedCorrectly_assignStringOfOneGreaterThanExceptionBufferCanContain_memberLenEqualsBufferSizeMinusTwo()
	{
		char buffer[OOLUA::ERROR::size+2];
		memset(buffer, 1, OOLUA::ERROR::size);
		buffer[OOLUA::ERROR::size+1]='\0';
		OOLUA::Exception e(buffer);
		size_t expected_value(OOLUA::ERROR::size-2);
		CPPUNIT_ASSERT_EQUAL(expected_value, e.m_len);
	}
	void exceptionBufferIsTrimmedCorrectly_assignStringOfOneGreaterThanExceptionBufferCanCotain_bufferMaxSizeMinusOneIsNullToken()
	{
		char buffer[OOLUA::ERROR::size+2];
		memset(buffer, 1, OOLUA::ERROR::size);
		buffer[OOLUA::ERROR::size+1]='\0';
		OOLUA::Exception e(buffer);
		char expected_value('\0');
		CPPUNIT_ASSERT_EQUAL(expected_value, e.m_buffer[OOLUA::ERROR::size-1]);
	}

	void exceptionPopsStack_pushStringToStackAndPassVmAndAskToPopTheStack_stackSizeIsZero()
	{
		lua_pushliteral(*m_lua, "DontCare");
		OOLUA::Exception e(*m_lua, static_cast<OOLUA::ERROR::PopTheStack*>(0));
		CPPUNIT_ASSERT_EQUAL(0, lua_gettop(*m_lua));
	}
	void exceptionDoesntPopStack_pushStringToStackAndPassVm_stackSizeIsOne()
	{
		lua_pushliteral(*m_lua, "DontCare");
		OOLUA::Exception e(*m_lua);
		CPPUNIT_ASSERT_EQUAL(1, lua_gettop(*m_lua));
	}
#endif

	bool isLuaJIT2()
	{
		/*
		jit.version_num
		Contains the version number of the LuaJIT core.
		Version xx.yy.zz is represented by the decimal number xxyyzz.
		*/
		m_lua->run_chunk("local res, ret = pcall( "
							"function() return require('jit').version_num >= 20000 end) "
						"return res == true and ret == true");
		bool result; m_lua->pull(result);
		return result;
	}

/* ====================== LuaJIT2 protected test ===========================*/
#if OOLUA_DEBUG_CHECKS == 1
#	if LUAJIT_VERSION_NUM >= 20000
		/*LuaJIT2 will throw on some platforms */
		void push_unregisteredClass_callsLuaPanic(){}
#	else
		/*If you are using LuaJIT2 with Lua headers this may still fail
		on platforms were LuaJIT2 uses exceptions by default.
		For example OSX X86_64.
		*/
		void push_unregisteredClass_callsLuaPanic()
		{
			if(isLuaJIT2()) return;

			Stub1 stubtmp;
			Stub1* stubptr(&stubtmp);

			lua_atpanic(*m_lua, &OOLua_panic);

			if (setjmp(mark) == 0)
			{
				OOLUA::push(*m_lua, stubptr);
				CPPUNIT_ASSERT_EQUAL(false, true);//never jumped back
			}
			else
				CPPUNIT_ASSERT_EQUAL(true, true);//we hit the at panic
		}

#	endif
#endif
/* ====================== LuaJIT2 protected test ===========================*/

	void canXmove_vm0IsNULL_returnsFalse()
	{
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::can_xmove(0, *m_lua));
	}

	void canXmove_vm1IsNULL_returnsFalse()
	{
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::can_xmove(*m_lua, 0));
	}

	void canXmove_samePointers_returnsFalse()
	{
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::can_xmove(*m_lua, *m_lua));
	}

	void canXmove_ParentAndChild_returnsTrue()
	{
		lua_State * child = lua_newthread(*m_lua);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::can_xmove(*m_lua, child));
	}

	void canXmove_unrelatedStates_returnsFalse()
	{
		lua_State * other = luaL_newstate();
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::can_xmove(*m_lua, other));
		lua_close(other);
	}

	void canXmove_ValidParentChildBothWithAStackCountOfOne_bothHaveStackCountIsOneAfterCall()
	{
		lua_State * child = lua_newthread(*m_lua);//thread on m_lua stack
		lua_pushnil(child);

		OOLUA::can_xmove(*m_lua, child);
		CPPUNIT_ASSERT(lua_gettop(*m_lua) == 1 && lua_gettop(child) == 1);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Error_test);
