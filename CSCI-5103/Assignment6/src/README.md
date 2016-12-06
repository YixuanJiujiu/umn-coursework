pungx013
assignment 7

I was not able to finish the assignment on time. Specifically, I did not construct any test cases. Although all the code I suspect I need to make the buffer driver work is in place, I have not ran it through any of the test cases. 

The only test case I have in place proves that the driver can be opened by "producers" and "consumers" from user space and the counters have the correct values. To test this, run the following commands from the root of the assignment directory...

	make
	./scull_load
	<< enter sudo password here >>
	<< dmesg here shows how much space was allocated for buffer >>
	gcc -Wall main.o main 
	./main
	<< dmesg here shows opening & closing of device >>
	./scull_unload
	rm main
