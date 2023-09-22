#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_QUESTIONS 500
#define MAX_OPTIONS 4
#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define PROGRESS_FILE "user_progress.txt"
#define USERS_FILE "users.txt"

typedef struct
{
    char question[200];
    char options[MAX_OPTIONS][100];
    char correctAnswer;
} Question;

typedef struct
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int quizzesTaken;
    float averageScore;
    int highestScore;
} User;

int numUsers = 0;

void swap(Question *a, Question *b)
{
    Question temp = *a;
    *a = *b;
    *b = temp;
}

void shuffleQuestions(Question questions[], int numQuestions)
{
    srand(time(NULL));
    for (int i = numQuestions - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        swap(&questions[i], &questions[j]);
    }
}

void printQuestion(Question q)
{
    printf("%s\n", q.question);
    for (int i = 0; i < MAX_OPTIONS; ++i)
    {
        printf("%c. %s\n", 'a' + i, q.options[i]);
    }
}

int loginUser(User users[], char *username, char *password)
{
    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
        {
            return i;
        }
    }
    return -1; // User not found or password incorrect
}

void registerUser(User users[], char *username, char *password)
{
    if (numUsers < MAX_USERS)
    {
        strcpy(users[numUsers].username, username);
        strcpy(users[numUsers].password, password);
        users[numUsers].quizzesTaken = 0;
        users[numUsers].averageScore = 0.0;
        users[numUsers].highestScore = 0;
        numUsers++;
        printf("Registration successful!\n");
    }
    else
    {
        printf("User limit reached. Cannot register.\n");
    }
}

void loadUsers(User users[])
{
    FILE *file = fopen(USERS_FILE, "r");
    if (file == NULL)
    {
        return; // Users file doesn't exist yet
    }

    while (fscanf(file, "%s %s %d %f %d\n", users[numUsers].username, users[numUsers].password,
                  &users[numUsers].quizzesTaken, &users[numUsers].averageScore, &users[numUsers].highestScore) == 5)
    {
        numUsers++;
    }

    fclose(file);
}

void saveUsers(User users[])
{
    FILE *file = fopen(USERS_FILE, "w");
    if (file == NULL)
    {
        printf("Error saving user data.\n");
        return;
    }

    for (int i = 0; i < numUsers; i++)
    {
        fprintf(file, "%s %s %d %.2f %d\n", users[i].username, users[i].password, users[i].quizzesTaken,
                users[i].averageScore, users[i].highestScore);
    }

    fclose(file);
}

void recordQuizResult(char *username, int score)
{
    FILE *file = fopen(PROGRESS_FILE, "a");
    if (file == NULL)
    {
        printf("Error recording quiz result.\n");
        return;
    }

    fprintf(file, "%s %d\n", username, score);
    fclose(file);
}

void calculateUserStats(User *user)
{
    FILE *file = fopen(PROGRESS_FILE, "r");
    if (file == NULL)
    {
        return; // No progress file yet
    }

    int totalScore = 0;
    int numQuizzes = 0;
    int highestScore = 0;

    char username[MAX_USERNAME_LENGTH];
    int score;

    while (fscanf(file, "%s %d\n", username, &score) == 2)
    {
        if (strcmp(username, user->username) == 0)
        {
            totalScore += score;
            numQuizzes++;
            if (score > highestScore)
            {
                highestScore = score;
            }
        }
    }

    fclose(file);

    if (numQuizzes > 0)
    {
        user->quizzesTaken = numQuizzes;
        user->averageScore = (float)totalScore / numQuizzes;
        user->highestScore = highestScore;
    }
}

void printMainMenu()
{
    printf("\n**************************************\n");
    printf("*********** Quiz Program ************\n");
    printf("**************************************\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("3. Exit\n");
    printf("**************************************\n");
    printf("**************************************\n");
}

void printUserMenu()
{
    printf("            1. Play Quiz\n");
    printf("            2. View Progress\n");
    printf("            3. Logout\n");
    printf("            4. Exit\n");
}

int main()
{
    FILE *file;
    char filename[] = "QBank.txt";
    int numQuestions, score = 0;

    // Open the file
    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    // Read questions from the file
    Question questions[MAX_QUESTIONS];
    int i;
    for (i = 0; i < MAX_QUESTIONS; ++i)
    {
        if (fscanf(file, " %[^\n]", questions[i].question) == EOF)
        {
            break;
        }
        for (int j = 0; j < MAX_OPTIONS; ++j)
        {
            fscanf(file, " %[^\n]", questions[i].options[j]);
        }
        fscanf(file, " %c", &questions[i].correctAnswer);
    }
    fclose(file);

    numQuestions = i;

    // Load existing users
    User users[MAX_USERS];
    loadUsers(users);

    int currentUserIndex = -1;
    int retake = 0;

    while (1)
    {
        // Shuffle the questions before each quiz
        shuffleQuestions(questions, numQuestions);

        if (currentUserIndex == -1)
        {
            // Print the login or registration menu
            printMainMenu();
            int choice;
            printf("Enter your choice: ");
            scanf("%d", &choice);
            getchar(); // Consume newline character

            if (choice == 1)
            {
                char username[MAX_USERNAME_LENGTH];
                char password[MAX_PASSWORD_LENGTH];
                printf("Enter your username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = '\0'; // Remove newline character

                printf("Enter your password: ");
                fgets(password, sizeof(password), stdin);
                password[strcspn(password, "\n")] = '\0'; // Remove newline character

                currentUserIndex = loginUser(users, username, password);
                if (currentUserIndex == -1)
                {
                    printf("Login failed. Invalid username or password.\n");
                }
                else
                {
                    printf("Login successful!\n");
                    calculateUserStats(&users[currentUserIndex]);
                }
            }
            else if (choice == 2)
            {
                char username[MAX_USERNAME_LENGTH];
                char password[MAX_PASSWORD_LENGTH];
                printf("Enter a username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = '\0'; // Remove newline character

                // Check if the username is already taken
                int usernameTaken = 0;
                for (int i = 0; i < numUsers; i++)
                {
                    if (strcmp(users[i].username, username) == 0)
                    {
                        usernameTaken = 1;
                        break;
                    }
                }

                if (usernameTaken)
                {
                    printf("Username is already taken. Please choose another one.\n");
                }
                else
                {
                    printf("Enter a password: ");
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = '\0'; // Remove newline character

                    registerUser(users, username, password);
                    calculateUserStats(&users[numUsers - 1]);
                    currentUserIndex = numUsers - 1;
                }
            }
            else if (choice == 3)
            {
                saveUsers(users);
                break; // Exit the program
            }
            else
            {
                printf("Invalid choice. Please enter 1, 2, or 3.\n");
            }
        }
        else
        {
            // Print the user-logged-in menu
            printUserMenu();
            int choice;
            printf("Enter your choice: ");
            scanf("%d", &choice);
            getchar(); // Consume newline character

            if (choice == 1)
            {
                // Prompt the user for the number of questions to play
                int numToPlay;
                do
                {
                    printf("\nHow many questions do you want to play? (Max %d): ", numQuestions);
                    scanf("%d", &numToPlay);
                    if (numToPlay > numQuestions)
                    {
                        printf("Invalid choice!\n");
                    }
                }
                while (numToPlay > numQuestions);

                // Start the quiz
                printf("\n-----Quiz Start!-----\n\n");
                for (i = 0; i < numToPlay; ++i)
                {
                    printf("%d. ", i + 1);
                    printQuestion(questions[i]);

                    char userAnswer;
                    printf("Your Answer: ");
                    scanf(" %c", &userAnswer);

                    if (userAnswer == questions[i].correctAnswer)
                    {
                        printf("\033[0;32mCorrect!\n\n\033[0m");
                        ++score;
                    }
                    else
                    {
                        printf("\033[0;31mWrong! The correct answer is: %c\n\n\033[0m", questions[i].correctAnswer);
                    }
                }

                // Record the quiz result
                int quizScore = (score * 100) / numToPlay;
                recordQuizResult(users[currentUserIndex].username, quizScore);
                users[currentUserIndex].quizzesTaken++;
                users[currentUserIndex].averageScore =
                    (users[currentUserIndex].averageScore * (users[currentUserIndex].quizzesTaken - 1) + quizScore) /
                    users[currentUserIndex].quizzesTaken;

                if (quizScore > users[currentUserIndex].highestScore)
                {
                    users[currentUserIndex].highestScore = quizScore; // Update the highest score
                }

                saveUsers(users);
                printf("\033[0;33mYour score: %d/%d\n\033[0m", score, numToPlay);

                if (quizScore < 40)
                {
                    printf("\033[0;31mYour score is less than 40%%. Do you want to retake the quiz? (1 for Yes, 0 for No): \033[0m");
                    scanf("%d", &retake);
                    if (retake == 0)
                    {
                        currentUserIndex = -1; // Logout
                        score = 0;
                    }
                    else
                    {
                        score = 0; // Reset score for retake
                    }
                }
                else
                {   
                    printf("\033[0;32mCongratulations! You passed the quiz.\n\n\033[0m");
                    score = 0; // Reset score
                }
            }
            else if (choice == 2)
            {
                saveUsers(users);
                printf("\nQuizzes Taken: %d\n", users[currentUserIndex].quizzesTaken);
                printf("Average Score: %.2f%%\n", users[currentUserIndex].averageScore);
                printf("Highest Score: %d%%\n\n", users[currentUserIndex].highestScore);
            }
            else if (choice == 3)
            {
                saveUsers(users);
                currentUserIndex = -1; // Logout
                score = 0;
            }
            else if (choice == 4)
            {
                saveUsers(users);
                break; // Exit the program
            }
            else
            {
                printf("Invalid choice. Please enter 1, 2, 3, or 4.\n");
            }
        }
    }

    // Save user data before exiting
    saveUsers(users);

    return 0;
}
