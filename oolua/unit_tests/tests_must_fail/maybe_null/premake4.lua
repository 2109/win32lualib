--Compile time tests for lua_out_p that must fail

local root = "../../../"

local create_compile_failing_test = function(source_name)

	create_package('maybe_null_'..source_name,root,'ConsoleApp')
	configuration {}
	files{ source_name..'.cpp' }
	includedirs
	{
		root .. 'include/',
		root .. 'unit_tests/bind_classes',
		root .. 'unit_tests/cpp_classes'
	}
	links{ "oolua" }

end

create_compile_failing_test("int")
create_compile_failing_test("stub")
create_compile_failing_test("ref_stub")
create_compile_failing_test("ref_const_stub")
create_compile_failing_test("ref_ptr_stub")
create_compile_failing_test("ref_const_ptr_stub")
create_compile_failing_test("ref_const_ptr_const_stub")
create_compile_failing_test("ref_ptr_const_stub")
