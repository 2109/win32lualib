cd ..
premake4 clean >/dev/null 2>&1
premake4 gmake linux >/dev/null 2>&1


#param 1: test_name
#only outputs a message when compiliation passes
function run_test()
{
if make $1 >/dev/null 2>&1; then
	echo "[failed] $1"
#else
#	echo "[pass] $1"
fi
}

run_test cpp_acquire_int
run_test cpp_acquire_ptr_int
run_test cpp_acquire_stub
run_test cpp_acquire_ref_stub

run_test lua_acquire_int
run_test lua_acquire_ref_int
run_test lua_acquire_ptr_int
run_test lua_acquire_ref_ptr_int
run_test lua_acquire_ptr_const_int
run_test lua_acquire_const_int
run_test lua_acquire_ref_const_int
run_test lua_acquire_const_ptr_const_int
run_test lua_acquire_ref_const_ptr_const_int
run_test lua_acquire_stub
run_test lua_acquire_ref_stub

run_test cpp_in_int
run_test cpp_in_ptr_int
run_test cpp_in_ref_int
run_test cpp_in_stub
run_test cpp_in_ref_stub
run_test cpp_in_ref_const_stub

run_test in_out_int
run_test in_out_stub

run_test lua_out_int
run_test lua_out_ref_int
run_test lua_out_ptr_int
run_test lua_out_ref_stub
run_test lua_out_ptr_stub
run_test lua_out_ref_const_stub
run_test lua_out_ref_ptr_const_stub

run_test lua_return_int
run_test lua_return_ptr_int
run_test lua_return_stub
run_test lua_return_const_stub
run_test lua_return_ref_stub
run_test lua_return_ref_const_stub

run_test out_int
run_test out_stub

run_test maybe_null_int
run_test maybe_null_stub
run_test maybe_null_ref_stub
run_test maybe_null_ref_const_stub
run_test maybe_null_ref_ptr_stub
run_test maybe_null_ref_const_ptr_stub
run_test maybe_null_ref_const_ptr_const_stub
run_test maybe_null_ref_ptr_const_stub

run_test lua_maybe_null_int
run_test lua_maybe_null_stub
run_test lua_maybe_null_ref_stub
run_test lua_maybe_null_ref_const_stub
run_test lua_maybe_null_ref_ptr_stub
run_test lua_maybe_null_ref_const_ptr_stub
run_test lua_maybe_null_ref_const_ptr_const_stub
run_test lua_maybe_null_ref_ptr_const_stub

premake4 clean >/dev/null 2>&1
cd build_scripts
