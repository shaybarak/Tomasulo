..\..\Tomasulo\Release\simulator.exe testdata\cmd_file.txt testdata\config_file.txt testdata\mem_init.txt testdata\regs_dump_actual.txt testdata\mem_dump_actual.txt testdata\time_actual.txt testdata\committed_actual.txt
@echo off
fc testdata\regs_dump_expected.txt testdata\regs_dump_actual.txt
fc testdata\mem_dump_expected.txt testdata\mem_dump_actual.txt
fc testdata\time_expected.txt testdata\time_actual.txt
fc testdata\committed_expected.txt testdata\committed_actual.txt
