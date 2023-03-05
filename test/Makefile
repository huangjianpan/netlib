test_utils_file_directory:
	rm -rf core*
	rm -rf main
	g++ -Wall -O2 -pthread -I ../ test_utils_file_directory.cpp ../utils/file.cpp ../utils/fmt.cpp -o main

test_log_log_data:
	rm -rf core*
	rm -rf main
	g++ -Wall -O2 -pthread -I ../ test_log_log_data.cpp ../log/*.cpp ../utils/*.cpp -o main

test_log_logger:
	rm -rf core*
	rm -rf main
	g++ -Wall -O2 -pthread -I ../ test_log_logger.cpp ../log/*.cpp ../utils/*.cpp -o main

test_utils_blocked_queue:
	rm -rf core*
	rm -rf main
	g++ -Wall -O2 -pthread -I ../ test_utils_blocked_queue.cpp -o main

test_context_context:
	rm -rf core*
	rm -rf main
	g++ -Wall -O2 -pthread -I ../ test_context_context.cpp ../context/*.cpp -o main

test_task_coroutine_task_context:
	rm -rf core*
	rm -rf main
	g++ -Wall -O2 -pthread -fstrict-aliasing -Wno-invalid-offsetof -Wno-unused-parameter -fno-omit-frame-pointer -I ../ -I ../task_coroutine test_task_coroutine_task_context.cpp ../task_coroutine/task_context.cpp -o main

test_task_coroutine:
	rm -rf core*
	rm -rf main
	g++ -Wall -g -pthread -fstrict-aliasing -Wno-invalid-offsetof -Wno-unused-parameter -fno-omit-frame-pointer -I ../ -I ../task_coroutine test_task_coroutine.cpp ../task_coroutine/*.cpp -o main

test_task_scheduling_queue:
	rm -rf main
	g++ -Wall -pthread -I ../task_coroutine test_task_scheduling_queue.cpp -o main

test_spin_mutex:
	rm -rf main
	g++ -Wall -pthread -I ../utils test_spin_mutex.cpp -o main