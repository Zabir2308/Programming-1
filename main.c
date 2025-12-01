#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>  // For trigonometric functions
#include "stack.h"  // Include the stack header file


#ifndef M_PI
    #define M_PI 3.14159265358979323846264338327950288419716939937510 // Pi constant from https://www.piday.org/million/
#endif
long double string_to_long_double(const char *str) {
    return strtold(str, NULL);  // string to long double
}
long double factorial(long double n) {
    if (n < 0) {
        printf("Error negative number");
         exit(1);
    }
    long double result = 1;
     for (long long i = 1; i <= n; i++) {
        result *= i;
    }//looop for factorial
      return result;
}
void rpn_calculator(char *expr) {
    char *token = strtok(expr, " ");  // Split input by spaces( 4 5 + )
    long double num1, num2;
    while (token != NULL) {
        // If the token is a number, push it onto the stack
        if (isdigit((unsigned char)token[0]) || (token[0] == '-' && isdigit((unsigned char)token[1]))) {
            push(string_to_long_double(token));
        } 
        // If  trig function (sin,tan)
        else if (strcmp(token, "sin") == 0 || strcmp(token, "cos") == 0 || strcmp(token, "tan") == 0) {
            if (isEmpty()) {
                printf("Error: Not enough operands for '%s'.\n", token);
                return;
            }

            num1 = pop();  // Pop the top operand (in degrees)
            double result;

            if (strcmp(token, "sin") == 0) {
                result = sin(num1 * M_PI / 180);  // Convert to radians and calculate sine
            } else if (strcmp(token, "cos") == 0) {
                result = cos(num1 * M_PI / 180);  // Convert to radians and calculate cosine
            } else if (strcmp(token, "tan") == 0) {
                result = tan(num1 * M_PI / 180);  // Convert to radians and calculate tangent
            }

            push(result);  // Push the result of the trigonometric function
        } 
        // If the token is an operator (+, -, *, /)
        else {
            if (isEmpty()) {
                printf("Error: Not enough operands for '%s'.\n", token);
                return;
            }

            num2 = pop();  // Pop the top operand
            if (isEmpty()) {
                printf("Error: Not enough operands for '%s'.\n", token);
                return;
            }

            num1 = pop();  // Pop the next operand

            // Perform the operation based on the operator
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
                        return;
                    }
                    break;
                default:
                    printf("Error: Invalid operator '%s'.\n", token);
                    return;
            }
        }

        token = strtok(NULL, " "); 
    }
    if (!isEmpty()) {
        long double result = pop();  // Pop the result
        if (isEmpty()) {
            printf("Final Result: %.15Lf\n", result);  // Display the result with precision
        } else {
            printf("Error: Too many values remaining on the stack.\n");
        }
    } else {
        printf("Error: Not enough values on the stack.\n");
    }
}

int main(void) {
    char expr[200];  // Size for input expression (you can modify this based on need)

    printf("Enter the calculation (must press space between operands and operators): ");
    if (fgets(expr, sizeof(expr), stdin) == NULL) {
        printf("Input error.\n");
        return 1;
    }

    expr[strcspn(expr, "\n")] = '\0';  // Remove the newline character

    rpn_calculator(expr);  // Perform RPN calculation

    return 0;
}



