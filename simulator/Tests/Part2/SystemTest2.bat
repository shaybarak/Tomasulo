..\..\..\Tomasulo\Release\simulator.exe testdata2\cmd_file.txt testdata2\config_file.txt testdata2\mem_init.txt testdata2\regs_dump_actual.txt testdata2\mem_dump_actual.txt testdata2\time_actual.txt testdata2\committed_actual.txt testdata2\hitrate_actual.txt testdata2\L1i_actual.txt testdata2\L1d_actual.txt testdata2\L2i_actual.txt testdata2\L2d_actual.txt
@echo off
fc testdata2\regs_dump_expected.txt testdata2\regs_dump_actual.txt
fc testdata2\mem_dump_expected.txt testdata2\mem_dump_actual.txt
fc testdata2\time_expected.txt testdata2\time_actual.txt
fc testdata2\committed_expected.txt testdata2\committed_actual.txt
fc testdata2\hitrate_expected.txt testdata2\hitrate_actual.txt
fc testdata2\L1i_expected.txt testdata2\L1i_actual.txt
fc testdata2\L1d_expected.txt testdata2\L1d_actual.txt
fc testdata2\L2i_expected.txt testdata2\L2i_actual.txt
fc testdata2\L2d_expected.txt testdata2\L2d_actual.txt
