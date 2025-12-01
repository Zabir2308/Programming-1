#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "stack.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846264338327950288419716939937510
#endif

#define HISTORY_FILE "history.txt"
#define MAX_HISTORY_DISPLAY 10

// Initial buffer sizes
#define INITIAL_LINE_SIZE 128
#define INITIAL_EXPR_SIZE 256
#define INITIAL_HISTORY_LINE_SIZE 512
#define INITIAL_HISTORY_ARRAY_SIZE 16

long double string_to_long_double(const char *str) {
    return strtold(str, NULL);
}

long double factorial(long double n) {
    if (n < 0) {
        printf("Error negative number");
        exit(1);
    }
    long double result = 1;
    for (long long i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Helper function to trim whitespace from both ends of a string
void trim_whitespace(char *str) {
    char *end;
    char *start = str;
    
    // Trim leading space
    while(isspace((unsigned char)*start)) start++;
    
    if(*start == 0) {
        str[0] = '\0';
        return;
    }
    
    // Trim trailing space
    end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    
    // Move trimmed string to beginning
    memmove(str, start, strlen(start) + 1);
}

// Dynamic string reading function - reads a complete line and grows buffer as needed
char* read_line_dynamic(FILE *stream, size_t initial_size) {
    size_t capacity = initial_size;
    size_t length = 0;
    char *buffer = (char *)malloc(capacity);
    
    if (buffer == NULL) {
        printf("Memory allocation error.\n");
        return NULL;
    }
    
    int c;
    while ((c = fgetc(stream)) != EOF) {
        // Check if we need more space (leave room for character + null terminator)
        if (length + 1 >= capacity) {
            capacity *= 2;
            char *new_buffer = (char *)realloc(buffer, capacity);
            if (new_buffer == NULL) {
                printf("Memory reallocation error.\n");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        
        // Stop at newline
        if (c == '\n') {
            buffer[length] = '\0';
            return buffer;
        }
        
        buffer[length++] = c;
    }
    
    // Handle EOF
    if (length == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }
    
    buffer[length] = '\0';
    return buffer;
}

// Check if string ends with '/' (ignoring whitespace)
int ends_with_continuation(char *str) {
    trim_whitespace(str);
    int len = strlen(str);
    
    if (len == 0) return 0;
    
    // Check from end for '/'
    int i = len - 1;
    while (i >= 0 && str[i] == '/') {
        i--;
    }
    
    // If we found at least one '/', remove all of them
    if (i < len - 1) {
        str[i + 1] = '\0';
        trim_whitespace(str);
        
        // If nothing remains after removing '/', treat as no input
        if (strlen(str) == 0) {
            return 2; // Special case: only '/' was entered
        }
        return 1; // Continuation needed
    }
    
    return 0; // No continuation
}

// Free all remaining nodes in the stack
void free_stack() {
    while (!isEmpty()) {
        pop();
    }
}

// Save calculation to history file
void save_to_history(const char *expression, long double result) {
    FILE *file = fopen(HISTORY_FILE, "a");
    if (file == NULL) {
        printf("Warning: Could not open history file.\n");
        return;
    }
    
    fprintf(file, "%s = %.15Lf\n", expression, result);
    fclose(file);
}

// Display last N entries from history with dynamic memory
void show_history() {
    system("clear");
    
    printf("=== History ===\n");
    
    FILE *file = fopen(HISTORY_FILE, "r");
    if (file == NULL) {
        printf("No history available yet.\n\n");
    } else {
        // Dynamic array of string pointers
        size_t capacity = INITIAL_HISTORY_ARRAY_SIZE;
        size_t count = 0;
        char **lines = (char **)malloc(capacity * sizeof(char *));
        
        if (lines == NULL) {
            printf("Memory allocation error.\n");
            fclose(file);
            return;
        }
        
        // Read all lines dynamically
        char *line;
        while ((line = read_line_dynamic(file, INITIAL_HISTORY_LINE_SIZE)) != NULL) {
            // Check if we need to grow the array
            if (count >= capacity) {
                capacity *= 2;
                char **new_lines = (char **)realloc(lines, capacity * sizeof(char *));
                if (new_lines == NULL) {
                    printf("Memory reallocation error.\n");
                    // Free what we have so far
                    for (size_t i = 0; i < count; i++) {
                        free(lines[i]);
                    }
                    free(lines);
                    free(line);
                    fclose(file);
                    return;
                }
                lines = new_lines;
            }
            
            lines[count++] = line;
        }
        fclose(file);
        
        if (count == 0) {
            printf("No history available yet.\n\n");
        } else {
            // Display last MAX_HISTORY_DISPLAY entries
            size_t start = (count > MAX_HISTORY_DISPLAY) ? count - MAX_HISTORY_DISPLAY : 0;
            for (size_t i = start; i < count; i++) {
                printf("%s\n", lines[i]);
            }
            printf("\n");
        }
        
        // Free all allocated lines
        for (size_t i = 0; i < count; i++) {
            free(lines[i]);
        }
        free(lines);
    }
    
    // Wait for user to return
    while (1) {
        printf("1. Return\n");
        printf("Enter your choice: ");
        
        char *choice = read_line_dynamic(stdin, INITIAL_LINE_SIZE);
        if (choice == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        
        trim_whitespace(choice);
        
        if (strcmp(choice, "1") == 0) {
            free(choice);
            break;
        } else {
            printf("Invalid choice. Please try again.\n\n");
        }
        
        free(choice);
    }
}

// Modified RPN calculator that returns 1 on success, 0 on error
int rpn_calculator(char *expr, long double *result) {
    char *token = strtok(expr, " ");
    long double num1, num2;
    
    while (token != NULL) {
        if (isdigit((unsigned char)token[0]) || (token[0] == '-' && isdigit((unsigned char)token[1]))) {
            push(string_to_long_double(token));
        } 
        else if (strcmp(token, "sin") == 0 || strcmp(token, "cos") == 0 || strcmp(token, "tan") == 0) {
            if (isEmpty()) {
                printf("Error: Not enough operands for '%s'.\n", token);
                return 0;
            }

            num1 = pop();
            double res;

            if (strcmp(token, "sin") == 0) {
                res = sin(num1 * M_PI / 180);
            } else if (strcmp(token, "cos") == 0) {
                res = cos(num1 * M_PI / 180);
            } else if (strcmp(token, "tan") == 0) {
                res = tan(num1 * M_PI / 180);
            }

            push(res);
        } 
        else {
            if (isEmpty()) {
                printf("Error: Not enough operands for '%s'.\n", token);
                return 0;
            }

            num2 = pop();
            if (isEmpty()) {
                printf("Error: Not enough operands for '%s'.\n", token);
                return 0;
            }

            num1 = pop();

            switch (token[0]) {
                case '+':
                    push(num1 + num2);
                    break;
                case '-':
                    push(num1 - num2);
                    break;
                case '*':
                    push(num1 * num2);
                    break;
                case '/':
                    if (num2 != 0) {
                        push(num1 / num2);
                    } else {
                        printf("Error! Division by zero.\n");
                        return 0;
                    }
                    break;
                default:
                    printf("Error: Invalid operator '%s'.\n", token);
                    return 0;
            }
        }

        token = strtok(NULL, " ");
    }
    
    if (!isEmpty()) {
        *result = pop();
        if (isEmpty()) {
            printf("Final Result: %.15Lf\n", *result);
            return 1; // Success
        } else {
            printf("Error: Too many values remaining on the stack.\n");
            return 0;
        }
    } else {
        printf("Error: Not enough values on the stack.\n");
        return 0;
    }
}

// Append to dynamic string with automatic growth
int append_to_string(char **dest, size_t *capacity, const char *src) {
    size_t dest_len = strlen(*dest);
    size_t src_len = strlen(src);
    size_t needed = dest_len + src_len + 2; // +2 for space and null terminator
    
    // Grow if needed
    while (needed > *capacity) {
        *capacity *= 2;
        char *new_dest = (char *)realloc(*dest, *capacity);
        if (new_dest == NULL) {
            printf("Memory reallocation error.\n");
            return 0;
        }
        *dest = new_dest;
    }
    
    // Append with space if dest is not empty
    if (dest_len > 0 && src_len > 0) {
        strcat(*dest, " ");
    }
    strcat(*dest, src);
    
    return 1;
}

// Calculation mode with dynamic memory
void calculation_mode() {
    system("clear");
    
    printf("=== Calculation Mode ===\n");
    printf("Use '/' at the end of a line to continue on the next line\n");
    printf("Type \"exit\" to return to selection mode\n\n");
    
    while (1) {
        size_t expr_capacity = INITIAL_EXPR_SIZE;
        char *expr = (char *)malloc(expr_capacity);
        
        if (expr == NULL) {
            printf("Memory allocation error.\n");
            return;
        }
        
        expr[0] = '\0';
        int continuation = 0;
        
        do {
            printf("Enter the calculation (press \"exit\" to exit): ");
            
            char *line = read_line_dynamic(stdin, INITIAL_LINE_SIZE);
            if (line == NULL) {
                printf("Error reading input.\n");
                free(expr);
                return;
            }
            
            // Check for exit command
            trim_whitespace(line);
            if (strcmp(line, "exit") == 0) {
                free(line);
                free(expr);
                return; // Return to selection mode
            }
            
            int cont_status = ends_with_continuation(line);
            
            if (cont_status == 2) {
                // Only '/' was entered, ignore and continue
                free(line);
                continue;
            }
            
            // Append to accumulated expression
            if (!append_to_string(&expr, &expr_capacity, line)) {
                free(line);
                free(expr);
                return;
            }
            
            continuation = cont_status;
            free(line);
            
        } while (continuation);

        // Process the complete expression
        if (strlen(expr) > 0) {
            // Make a copy for history (strtok modifies the string)
            char *expr_copy = strdup(expr);
            if (expr_copy == NULL) {
                printf("Memory allocation error.\n");
                free(expr);
                continue;
            }
            
            long double result;
            if (rpn_calculator(expr, &result)) {
                // Only save to history on successful calculation
                save_to_history(expr_copy, result);
            }
            
            free(expr_copy);
            
            // Clear the stack for next calculation (in case of error)
            while (!isEmpty()) {
                pop();
            }
        }
        
        free(expr);
        printf("\n");
    }
}

// Selection mode with dynamic memory
void selection_mode() {
    while (1) {
        system("clear");
        
        printf("=== Selection Mode ===\n");
        printf("1. New calculation\n");
        printf("2. Show history\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        
        char *choice = read_line_dynamic(stdin, INITIAL_LINE_SIZE);
        if (choice == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        
        trim_whitespace(choice);
        
        if (strcmp(choice, "1") == 0) {
            free(choice);
            calculation_mode();
        } else if (strcmp(choice, "2") == 0) {
            free(choice);
            show_history();
        } else if (strcmp(choice, "3") == 0) {
            free(choice);
            free_stack();  // Free all remaining nodes before exit
            system("clear");
            printf("Exiting calculator. History saved to %s\n", HISTORY_FILE);
            exit(0);
        } else {
            printf("Invalid choice. Please try again.\n");
            printf("Press Enter to continue...");
            char *dummy = read_line_dynamic(stdin, INITIAL_LINE_SIZE);
            free(dummy);
        }
        
        free(choice);
    }
}

int main(void) {
    selection_mode();
    return 0;
}