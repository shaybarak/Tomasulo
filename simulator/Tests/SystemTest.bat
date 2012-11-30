..\..\Tomasulo\Release\simulator.exe cmd_file.txt config_file.txt mem_init.txt regs_dump_actual.txt mem_dump_actual.txt time_actual.txt committed_actual.txt
fc regs_dump_expected.txt regs_dump_actual.txt
fc mem_dump_expected.txt mem_dump_actual.txt
fc time_expected.txt time_actual.txt
fc committed_expected.txt committed_actual.txt
