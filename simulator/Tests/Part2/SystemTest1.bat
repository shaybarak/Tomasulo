..\..\..\Tomasulo\Release\simulator.exe testdata1\cmd_file.txt testdata1\config_file.txt testdata1\mem_init.txt testdata1\regs_dump_actual.txt testdata1\mem_dump_actual.txt testdata1\time_actual.txt testdata1\committed_actual.txt testdata1\hitrate_actual.txt testdata1\L1i_actual.txt testdata1\L1d_actual.txt testdata1\L2i_actual.txt testdata1\L2d_actual.txt
@echo off
fc testdata1\regs_dump_expected.txt testdata1\regs_dump_actual.txt
fc testdata1\mem_dump_expected.txt testdata1\mem_dump_actual.txt
fc testdata1\time_expected.txt testdata1\time_actual.txt
fc testdata1\committed_expected.txt testdata1\committed_actual.txt
fc testdata1\hitrate_expected.txt testdata1\hitrate_actual.txt
fc testdata1\L1i_expected.txt testdata1\L1i_actual.txt
fc testdata1\L1d_expected.txt testdata1\L1d_actual.txt
fc testdata1\L2i_expected.txt testdata1\L2i_actual.txt
fc testdata1\L2d_expected.txt testdata1\L2d_actual.txt
