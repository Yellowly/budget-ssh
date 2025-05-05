#include <cstdlib>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/tsh.h"

// Test fixture for TSH tests
class TSHTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test environment
        // Create test files with full content
        std::ofstream inputFile("test_input.txt");
        inputFile << "This is test input" << std::endl;
        inputFile.close();
        
        // Make sure the file exists and is readable
        std::ifstream checkFile("test_input.txt");
        ASSERT_TRUE(checkFile.good()) << "Failed to create test_input.txt";
        checkFile.close();
    }

    void TearDown() override {
        // Clean up test environment
        system("rm -f test_input.txt test_output.txt");
    }
    
    // Helper function to capture and test command output
    std::string captureCommandOutput(const char* command) {
        // Create pipes for communication
        int pipefd[2];
        if (pipe(pipefd) != 0) {
            return "Error creating pipe";
        }
        
        pid_t pid = fork();
        if (pid < 0) {
            close(pipefd[0]);
            close(pipefd[1]);
            return "Error forking process";
        }
        
        if (pid == 0) {
            // Child process
            close(pipefd[0]); // Close read end
            
            // Make sure we're in the right directory
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                fprintf(stderr, "Test running in directory: %s\n", cwd);
            }
            
            // Redirect stdout and stderr to pipe
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);
            
            // Debug info - list available files
            system("ls -la >> /tmp/test_debug.log 2>&1");
            
            // Create an isolated environment for the command execution
            ProcessChain process_list;
            parse_input(const_cast<char*>(command), &process_list);
            
            // Execute the command
            int result = run_commands(&process_list);
            if (result != 0) {
                fprintf(stderr, "Command execution failed with code %d\n", result);
            }
            
            // Free resources in the process_list
            Process* current = process_list.head;
            while (current) {
                Process* next = current->next;
                if (current->args) {
                    for (int i = 0; i < current->argc; i++) {
                        if (current->args[i]) free(current->args[i]);
                    }
                    free(current->args);
                }
                if (current->output_file) free(current->output_file);
                free(current);
                current = next;
            }
            
            // Make sure to flush any buffered output
            fflush(stdout);
            fflush(stderr);
            
            _exit(0);
        } else {
            // Parent process
            close(pipefd[1]); // Close write end
            
            // Read output from pipe with timeout
            char buffer[4096];
            std::string result;
            ssize_t bytes;
            
            // Set pipe to non-blocking
            int flags = fcntl(pipefd[0], F_GETFL, 0);
            fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
            
            // Wait for child with timeout - reduce timeout for faster test execution
            int status;
            pid_t wpid;
            time_t start_time = time(NULL);
            bool child_done = false;
            
            while (time(NULL) - start_time < 2) { // 2-second timeout (reduced from 3)
                // Check if child has exited
                wpid = waitpid(pid, &status, WNOHANG);
                if (wpid == pid) {
                    child_done = true;
                }
                
                // Try to read available data
                bytes = read(pipefd[0], buffer, sizeof(buffer) - 1);
                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    result += buffer;
                } else if (bytes == 0 && child_done) {
                    // End of file and child is done
                    break;
                }
                
                // If child is done but we're still reading, don't wait too long
                if (child_done) {
                    usleep(5000); // 5ms
                } else {
                    // Small sleep to prevent CPU thrashing
                    usleep(10000); // 10ms
                }
            }
            
            // If child hasn't exited yet, kill it
            if (!child_done) {
                kill(pid, SIGTERM);
                usleep(10000); // Give it a moment to terminate
                kill(pid, SIGKILL); // Force kill if still running
                waitpid(pid, &status, 0);
                result += "\n[Test timeout: command execution took too long]";
            }
            
            close(pipefd[0]);
            return result;
        }
    }
};

// Test basic command execution
TEST_F(TSHTest, BasicCommandExecution) {
    // Test basic ls command (may vary depending on environment)
    std::string output = captureCommandOutput("echo hello world");
    EXPECT_TRUE(output.find("hello world") != std::string::npos);
}

// Test pipe operator
TEST_F(TSHTest, PipeOperator) {
    std::string output = captureCommandOutput("echo hello world | grep hello");
    EXPECT_TRUE(output.find("hello") != std::string::npos);
}

// Test output redirection
TEST_F(TSHTest, OutputRedirection) {
    // Run command with output redirection
    captureCommandOutput("echo redirect test > test_output.txt");
    
    // Check if file was created and has correct content
    std::ifstream file("test_output.txt");
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("redirect test") != std::string::npos);
}

// Test input redirection
TEST_F(TSHTest, InputRedirection) {
    std::string output = captureCommandOutput("cat < test_input.txt");
    EXPECT_TRUE(output.find("This is test input") != std::string::npos);
}

// Test multiple commands with semicolon
TEST_F(TSHTest, SemicolonOperator) {
    std::string output = captureCommandOutput("echo first command ; echo second command");
    EXPECT_TRUE(output.find("first command") != std::string::npos);
    EXPECT_TRUE(output.find("second command") != std::string::npos);
}

// Test combining operators
TEST_F(TSHTest, CombinedOperators) {
    std::string output = captureCommandOutput("echo hello | grep hello > test_output.txt ; echo done");
    EXPECT_TRUE(output.find("done") != std::string::npos);
    
    // Check file content
    std::ifstream file("test_output.txt");
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("hello") != std::string::npos);
}

// Test background processing
// Note: This is trickier to test automatically since the process runs in background
TEST_F(TSHTest, BackgroundProcessing) {
    // This test is more of a demonstration than an actual automated test
    // In a real test environment, you would need more sophisticated methods to verify
    // background processes
    std::string output = captureCommandOutput("echo background test &");
    // At least verify the command doesn't error out
    EXPECT_FALSE(output.find("Error") != std::string::npos);
}