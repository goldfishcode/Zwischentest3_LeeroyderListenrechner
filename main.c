#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum node_type{
  number_type = 1, operator_type, parenthesis_type  
};

struct node {
    char operator;  
    int number;
    enum node_type type;
    struct node* next;
};

struct node* inputToNode(const char *input);

struct node* createParenthesisNode(char operator);
struct node* createOperatorNode(char operator);
struct node* createNumberNode(int number);

struct node* findLastParenthesisOpen(struct node* head);
struct node* findFirstPointOperator(struct node *start);
struct node* findFirstDashOperator(struct node *start);
struct node* findPrevious(struct node* head, struct node* node);

struct node* removeNode(struct node* head, struct node* node); // return: new head of the list

struct node* evalOperation(struct node* head, struct node* operator, struct node** start); // Double pointer to update beginning of search incase location is removed
struct node* evaluate(struct node* head, struct node** start, struct node* closing_parenthesis); // if closing_parenthesis = NULL, treated like a regular expression. Otherwise, only evaluates things within the parenthesis

struct node *inputToNode(const char *input)
{
    int number = atoi(input);
    if (number != 0)
        return createNumberNode(number);

    if (input[0] == '0')
        return createNumberNode(0);

    if (strcmp(input, "(") == 0 ||
        strcmp(input, ")") == 0)
        return createParenthesisNode(input[0]);

    if (strcmp(input, "+") == 0 ||
        strcmp(input, "-") == 0 ||
        strcmp(input, "*") == 0 ||
        strcmp(input, "/") == 0)
        return createOperatorNode(input[0]);

    return NULL;
}

struct node* createParenthesisNode(char operator)
{
	struct node* node = malloc(sizeof(struct node));
	node->operator = operator;
	node->number = 0;
	node->type = parenthesis_type;
	node->next = NULL;
	return node;
}

struct node* createOperatorNode(char operator)
{
	struct node* node = malloc(sizeof(struct node));
	node->operator = operator;
	node->number = 0;
	node->type = operator_type;
	node->next = NULL;
	return node;
}

struct node* createNumberNode(int number)
{
	struct node* node = malloc(sizeof(struct node));
	node->operator = '\0';
	node->number = number;
	node->type = number_type;
	node->next = NULL;
	return node;
}

struct node* findLastParenthesisOpen(struct node* head)
{
	struct node* node = NULL;
	while(head != NULL)
	{
		if(head->type == parenthesis_type && head->operator == '(')
			node = head;

		head = head->next;
	}

	return node;
}

struct node* findFirstPointOperator(struct node *head)
{
	while(head != NULL)
	{
		if(head->type == operator_type && (head->operator == '*' || head->operator == '/'))
			return head;

		head = head->next;
	}

	return NULL;
}

struct node* findFirstDashOperator(struct node *head)
{
	while(head != NULL)
	{
		if(head->type == operator_type && (head->operator == '+' || head->operator == '-'))
			return head;

		head = head->next;
	}

	return NULL;
}

struct node* findPrevious(struct node* head, struct node* node)
{
	while(head != NULL && head->next != node)
		head = head->next;

	return head;
}

// Returns new head of the list, in the case that head is removed
struct node* removeNode(struct node* head, struct node* node)
{
	struct node* prev = findPrevious(head, node);
	if(prev == NULL)
		head = node->next;
	else
		prev->next = node->next;
	
	free(node);
	return head;
}

// Double pointer start updates the start value in evaluate function.
// Returns new head of the list, in the case that head is removed
struct node* evalOperation(struct node* head, struct node* operator, struct node** start)
{
	int a, b; // the numbers to calculate the operation on
	struct node* prev = findPrevious(head, operator);
	struct node* next = operator->next;

	int update_start = 0;

	// If "a" is in parentheses, remove them from the linked list and set a properly
	if(prev->type == parenthesis_type)
	{
		// ( -> a -> ) -> operator

		struct node* num = findPrevious(head, prev);
		a = num->number;

		struct node* prev = findPrevious(head, num);
		if(prev == *start)
			update_start = 1;

		head = removeNode(head, prev);
		head = removeNode(head, num->next);
	}
	else
		a = prev->number;

	// Same as "a", but it is after the operator, instead of before it
	if(next->type == parenthesis_type)
	{
		// operator -> ( -> b -> )

		b = next->next->number;

		head = removeNode(head, next->next->next);
		head = removeNode(head, next->next);
	}
	else
		b = next->number;
	
	head = removeNode(head, next);

	switch(operator->operator)
	{
		case '+':
			a += b;
			break;
		case '-':
			a -= b;
			break;
		case '*':
			a *= b;
			break;
		case '/':
			if(b != 0) // ignore division by zero (treated as division by 1)
				a /= b;
			break;
	}

	// Number is stored in the first number node in the operation
	struct node* node = findPrevious(head, operator);
	node->number = a;

	head = removeNode(head, operator);

	if(update_start)
		*start = head;
	
	return head;
}

void printList(struct node* head)
{
	while(head != NULL)
	{
		if(head->type == number_type)
			printf("%d", head->number);
		else
			printf("%c", head->operator);

		head = head->next;
	}
}

// Recursive function. If root, closing parenthesis will be NULL. Otherwise, processses everything within the parenthesis
struct node* evaluate(struct node* head, struct node** start, struct node* closing_parenthesis)
{
	struct node* curr;
	if(closing_parenthesis == NULL) // if we aren't evaluating a parentheses expression
	{
		curr = findLastParenthesisOpen(head);
		while(curr != NULL) // loop through until no parenthesis are left to evaluate
		{
			struct node* closing = curr;
			
			// Find the closing parenthesis
			int num_open = 1;
			while(num_open > 0)
			{
				closing = closing->next;
				if(closing->operator == '(')
					num_open++;
				else if(closing->operator == ')')
					num_open--;
			}

			if(closing == curr->next->next)
			{
				head = removeNode(head, curr);
				head = removeNode(head, closing);
				*start = head;
			}
			else
				head = evaluate(head, &curr, closing); // Evaluate everything within the parentheses

			curr = findLastParenthesisOpen(head); // Update iterator
		}
	}

	curr = findFirstPointOperator(*start);
	while(curr != NULL) // Loop until no more point operations are left to evaluate
	{
		// Don't evaluate operations outside the parentheses if they exist
		if(closing_parenthesis != NULL && curr > closing_parenthesis)
			break;

		head = evalOperation(head, curr, start);
		printf("\nResulting term: ");
		printList(head);
		curr = findFirstPointOperator(*start);
	}

	curr = findFirstDashOperator(*start);
	while(curr != NULL) // Loop until no more dash operations are left to evaluate
	{
		// Don't evaluate operations outside the parentheses if they exist
		if(closing_parenthesis != NULL && curr > closing_parenthesis)
			break;

		head = evalOperation(head, curr, start);
		printf("\nResulting term: ");
		printList(head);
		curr = findFirstDashOperator(*start);
	}

	return head;
}

int main()
{
	struct node* head = NULL;
	char input[10];
	while(1)
	{
		printf("\nInput: ");
		scanf("%s", input);
		if(input[0] == '=')
			break;

		struct node* node = inputToNode(input);
		struct node* curr = head;

		// Print each term so far and get to end of the list
		printf("\nTerm: ");
		while(curr != NULL && curr->next != NULL)
		{
			if(curr->type == number_type)
				printf("%d", curr->number);
			else
				printf("%c", curr->operator);

			curr = curr->next;
		}

		// insert node into end of the list and print previous node
		if(curr == NULL)
			head = node;
		else
		{
			if(curr->type == number_type)
				printf("%d", curr->number);
			else
				printf("%c", curr->operator);

			curr->next = node;
		}
		
		// print new node
		if(node->type == number_type)
			printf("%d", node->number);
		else
			printf("%c", node->operator);

	}

	struct node* prev;
	// evaluate the expression, starting at the head of the list
	evaluate(head, &head, NULL);

	// free the list
	while(head != NULL)
	{
		prev = head;
		head = head->next;
		free(prev);
	}

	return 0;
}
