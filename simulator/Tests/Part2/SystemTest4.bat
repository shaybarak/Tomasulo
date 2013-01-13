..\..\..\Tomasulo\Release\simulator.exe testdata4\cmd_file.txt testdata4\config_file.txt testdata4\mem_init.txt testdata4\regs_dump_actual.txt testdata4\mem_dump_actual.txt testdata4\time_actual.txt testdata4\committed_actual.txt testdata4\hitrate_actual.txt testdata4\L1i_actual.txt testdata4\L1d_actual.txt testdata4\L2i_actual.txt testdata4\L2d_actual.txt
@echo off
fc testdata4\regs_dump_expected.txt testdata4\regs_dump_actual.txt
fc testdata4\mem_dump_expected.txt testdata4\mem_dump_actual.txt
fc testdata4\time_expected.txt testdata4\time_actual.txt
fc testdata4\committed_expected.txt testdata4\committed_actual.txt
fc testdata4\hitrate_expected.txt testdata4\hitrate_actual.txt
fc testdata4\L1i_expected.txt testdata4\L1i_actual.txt
fc testdata4\L1d_expected.txt testdata4\L1d_actual.txt
fc testdata4\L2i_expected.txt testdata4\L2i_actual.txt
fc testdata4\L2d_expected.txt testdata4\L2d_actual.txt
