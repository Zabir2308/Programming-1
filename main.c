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
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) {
        str[0] = '\0';
        return;
    }
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    
    // Move trimmed string to beginning
    memmove(str - (str - str), str, strlen(str) + 1);
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

// Display last N entries from history
void show_history() {
    system("clear");
    
    printf("=== History ===\n");
    
    FILE *file = fopen(HISTORY_FILE, "r");
    if (file == NULL) {
        printf("No history available yet.\n\n");
    } else {
        char lines[1000][300]; // Store up to 1000 lines
        int count = 0;
        
        // Read all lines
        while (fgets(lines[count], sizeof(lines[count]), file) != NULL && count < 1000) {
            count++;
        }
        fclose(file);
        
        if (count == 0) {
            printf("No history available yet.\n\n");
        } else {
            // Display last MAX_HISTORY_DISPLAY entries
            int start = (count > MAX_HISTORY_DISPLAY) ? count - MAX_HISTORY_DISPLAY : 0;
            for (int i = start; i < count; i++) {
                printf("%s", lines[i]);
            }
            printf("\n");
        }
    }
    
    // Wait for user to return
    while (1) {
        printf("1. Return\n");
        printf("Enter your choice: ");
        
        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            continue;
        }
        
        choice[strcspn(choice, "\n")] = '\0';
        trim_whitespace(choice);
        
        if (strcmp(choice, "1") == 0) {
            break;
        } else {
            printf("Invalid choice. Please try again.\n\n");
        }
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

// Calculation mode
void calculation_mode() {
    system("clear");
    
    printf("=== Calculation Mode ===\n");
    printf("Use '/' at the end of a line to continue on the next line\n");
    printf("Type \"exit\" to return to selection mode\n\n");
    
    while (1) {
        char expr[1000] = "";
        char expr_copy[1000] = "";
        char line[200];
        int continuation = 0;
        
        // Reset expression for new calculation
        expr[0] = '\0';
        
        do {
            printf("Enter the calculation (press \"exit\" to exit): ");
            
            if (fgets(line, sizeof(line), stdin) == NULL) {
                printf("Unexpected error.\n");
                return;
            }

            line[strcspn(line, "\n")] = '\0';  // Remove newline
            
            // Check for exit command
            trim_whitespace(line);
            if (strcmp(line, "exit") == 0) {
                return; // Return to selection mode
            }
            
            int cont_status = ends_with_continuation(line);
            
            if (cont_status == 2) {
                // Only '/' was entered, ignore and continue
                continue;
            }
            
            // Append to accumulated expression with a space
            if (strlen(expr) > 0 && strlen(line) > 0) {
                strcat(expr, " ");
            }
            strcat(expr, line);
            
            continuation = cont_status;
            
        } while (continuation);

        // Process the complete expression
        if (strlen(expr) > 0) {
            // Make a copy for history (strtok modifies the string)
            strcpy(expr_copy, expr);
            
            long double result;
            if (rpn_calculator(expr, &result)) {
                // Only save to history on successful calculation
                save_to_history(expr_copy, result);
            }
            
            // Clear the stack for next calculation (in case of error)
            while (!isEmpty()) {
                pop();
            }
        }
        
        printf("\n");
    }
}

// Selection mode
void selection_mode() {
    while (1) {
        system("clear");
        
        printf("=== Selection Mode ===\n");
        printf("1. New calculation\n");
        printf("2. Show history\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        
        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            printf("Unexpected error.\n");
            continue;
        }
        
        choice[strcspn(choice, "\n")] = '\0';
        trim_whitespace(choice);
        
        if (strcmp(choice, "1") == 0) {
            calculation_mode();
        } else if (strcmp(choice, "2") == 0) {
            show_history();
        } else if (strcmp(choice, "3") == 0) {
            free_stack();  // Free all remaining nodes before exit
            system("clear");
            printf("Exiting calculator. History saved to %s\n", HISTORY_FILE);
            exit(0);
        } else {
            printf("Invalid choice. Please try again.\n");
            printf("Press Enter to continue...");
            getchar();
        }
    }
}

int main(void) {
    selection_mode();
    return 0;
}