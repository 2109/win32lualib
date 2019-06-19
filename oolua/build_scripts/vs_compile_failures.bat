@echo OFF
cd ..

if not exist .\build_logs  md .\build_logs

call premake4 clean > NUL
call premake4 vs%1 windows > NUL

set testPrefix=.\build_scripts\vs_run_test.bat debug tests_must_fail\

call %testPrefix%cpp_acquire\cpp_acquire_int cpp_acquire_int %1 fail
call %testPrefix%cpp_acquire\cpp_acquire_ptr_int cpp_acquire_ptr_int %1 fail 
call %testPrefix%cpp_acquire\cpp_acquire_stub cpp_acquire_stub %1 fail 
call %testPrefix%cpp_acquire\cpp_acquire_ref_stub cpp_acquire_ref_stub %1 fail 


call %testPrefix%lua_acquire\lua_acquire_int lua_acquire_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ref_int lua_acquire_ref_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ptr_int lua_acquire_ptr_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ref_ptr_int lua_acquire_ref_ptr_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ptr_const_int lua_acquire_ptr_const_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_const_int lua_acquire_const_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ref_const_int lua_acquire_ref_const_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_const_ptr_const_int lua_acquire_const_ptr_const_int %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ref_const_ptr_const_int lua_acquire_ref_const_ptr_const_int %1 fail  
call %testPrefix%lua_acquire\lua_acquire_stub lua_acquire_stub %1 fail 
call %testPrefix%lua_acquire\lua_acquire_ref_stub lua_acquire_ref_stub %1 fail 


call %testPrefix%cpp_in\cpp_in_int cpp_in_int %1 fail 
call %testPrefix%cpp_in\cpp_in_ptr_int cpp_in_ptr_int %1 fail 
call %testPrefix%cpp_in\cpp_in_ref_int cpp_in_ref_int %1 fail 
call %testPrefix%cpp_in\cpp_in_stub cpp_in_stub %1 fail 
call %testPrefix%cpp_in\cpp_in_ref_stub cpp_in_ref_stub %1 fail 
call %testPrefix%cpp_in\cpp_in_ref_const_stub cpp_in_ref_const_stub %1 fail 


call %testPrefix%in_out\in_out_int in_out_int %1 fail 
call %testPrefix%in_out\in_out_stub in_out_stub %1 fail 


call %testPrefix%lua_out\lua_out_int lua_out_int %1 fail 
call %testPrefix%lua_out\lua_out_ref_int lua_out_ref_int %1 fail 
call %testPrefix%lua_out\lua_out_ptr_int lua_out_ptr_int %1 fail 
call %testPrefix%lua_out\lua_out_ref_stub lua_out_ref_stub %1 fail 
call %testPrefix%lua_out\lua_out_ptr_stub lua_out_ptr_stub %1 fail 
call %testPrefix%lua_out\lua_out_ref_const_stub lua_out_ref_const_stub %1 fail 
call %testPrefix%lua_out\lua_out_ref_ptr_const_stub lua_out_ref_ptr_const_stub %1 fail 


call %testPrefix%lua_return\lua_return_int lua_return_int %1 fail 
call %testPrefix%lua_return\lua_return_ptr_int lua_return_ptr_int %1 fail 
call %testPrefix%lua_return\lua_return_stub lua_return_stub %1 fail 
call %testPrefix%lua_return\lua_return_const_stub lua_return_const_stub %1 fail 
call %testPrefix%lua_return\lua_return_ref_stub lua_return_ref_stub %1 fail 
call %testPrefix%lua_return\lua_return_ref_const_stub lua_return_ref_const_stub %1 fail 


call %testPrefix%out\out_int out_int %1 fail 
call %testPrefix%out\out_stub out_stub %1 fail 


call %testPrefix%maybe_null\maybe_null_int maybe_null_int %1 fail 
call %testPrefix%maybe_null\maybe_null_stub maybe_null_stub %1 fail 
call %testPrefix%maybe_null\maybe_null_ref_stub maybe_null_ref_stub %1 fail 
call %testPrefix%maybe_null\maybe_null_ref_const_stub maybe_null_ref_const_stub %1 fail 
call %testPrefix%maybe_null\maybe_null_ref_ptr_stub maybe_null_ref_ptr_stub %1 fail 
call %testPrefix%maybe_null\maybe_null_ref_const_ptr_stub maybe_null_ref_const_ptr_stub %1 fail 
call %testPrefix%maybe_null\maybe_null_ref_const_ptr_const_stub maybe_null_ref_const_ptr_const_stub %1 fail 
call %testPrefix%maybe_null\maybe_null_ref_ptr_const_stub maybe_null_ref_ptr_const_stub %1 fail 


call %testPrefix%lua_maybe_null\lua_maybe_null_int lua_maybe_null_int %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_stub lua_maybe_null_stub %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_ref_stub lua_maybe_null_ref_stub %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_ref_const_stub lua_maybe_null_ref_const_stub %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_ref_ptr_stub lua_maybe_null_ref_ptr_stub %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_ref_const_ptr_stub lua_maybe_null_ref_const_ptr_stub %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_ref_const_ptr_const_stub lua_maybe_null_ref_const_ptr_const_stub %1 fail 
call %testPrefix%lua_maybe_null\lua_maybe_null_ref_ptr_const_stub lua_maybe_null_ref_ptr_const_stub %1 fail 


call premake4 clean
cd build_scripts
