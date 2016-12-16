#!/usr/bin/python
import glob


netlist_path = '../mcac/netlist/mcac_tsmc065_scan/*.vs'

keywords = ['boot_mem128_0_test_1 ', 'boot_mem128_1_test_1 ', 'enc_WB_DWIDTH128_WB_SWIDTH16_WB_SLAVES10_test_1 ', 'dec_WB_DWIDTH128_WB_SWIDTH16_WB_SLAVES10_test_1 ']

change_dict = {
	'boot_mem128_0_test_1 ' : 'boot_mem128 ',
	'boot_mem128_1_test_1 ' : 'boot_mem128 ',
	'enc_WB_DWIDTH128_WB_SWIDTH16_WB_SLAVES10_test_1 ' : 'enc ',
	'dec_WB_DWIDTH128_WB_SWIDTH16_WB_SLAVES10_test_1 ' : 'dec '
}

new_lines = []

file_list = glob.glob(netlist_path)

#print file_list

for i in range(0, len(file_list), 1):
	file_reader = open(file_list[i],'r')
	new_file_name = file_list[i].strip()
	for key in range(0, len(change_dict), 1):
		file_name = file_list[i].strip()
		if (keywords[key].strip() in file_name):
			new_file_name = file_name[0:file_name.rfind('/')+1] + change_dict[keywords[key]].strip() + '.vs'
	for line in file_reader:
		write_flag = 0
		for key in range(0, len(change_dict), 1):
			if (keywords[key] in line):
				start_index = line.index(keywords[key])
				end_index = start_index + len(keywords[key])
				new_line = line[0:start_index] + change_dict[keywords[key]] + line[end_index:]
				write_flag = 1
			elif (write_flag == 0):
				new_line = line
		new_lines.append(new_line)
	file_reader.close()

	file_writer = open(new_file_name,'w')
	for n in range(0, len(new_lines), 1):
		file_writer.write(new_lines[n])
	file_writer.close()
	new_lines = []
		









