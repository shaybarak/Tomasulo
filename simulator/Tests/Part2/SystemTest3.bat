..\..\..\Tomasulo\Release\simulator.exe testdata3\cmd_file.txt testdata3\config_file.txt testdata3\mem_init.txt testdata3\regs_dump_actual.txt testdata3\mem_dump_actual.txt testdata3\time_actual.txt testdata3\committed_actual.txt testdata3\hitrate_actual.txt testdata3\L1i_actual.txt testdata3\L1d_actual.txt testdata3\L2i_actual.txt testdata3\L2d_actual.txt
@echo off
fc testdata3\regs_dump_expected.txt testdata3\regs_dump_actual.txt
fc testdata3\mem_dump_expected.txt testdata3\mem_dump_actual.txt
fc testdata3\time_expected.txt testdata3\time_actual.txt
fc testdata3\committed_expected.txt testdata3\committed_actual.txt
fc testdata3\hitrate_expected.txt testdata3\hitrate_actual.txt
fc testdata3\L1i_expected.txt testdata3\L1i_actual.txt
fc testdata3\L1d_expected.txt testdata3\L1d_actual.txt
fc testdata3\L2i_expected.txt testdata3\L2i_actual.txt
fc testdata3\L2d_expected.txt testdata3\L2d_actual.txt
