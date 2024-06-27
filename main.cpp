#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

typedef enum { PIRATES, CRIME_DRAMA, COMICS } Theme;

typedef struct {
    const char* word;
    const char* hint;
} WordHintPair;

// Word and hint pairs for the pirates theme
const WordHintPair piratesWords[] = {
    {"treasure", "Pirate's gold"},
    {"ship", "Pirate's vehicle"},
    {"parrot", "Pirate's pet"},
    {"captain", "Leader of the pirates"},
    {"island", "Pirate's hideout"}
};

// Word and hint pairs for the crime drama theme
const WordHintPair crimeDramaWords[] = {
    {"detective", "Investigative professional"},
    {"murder", "Intentional killing of a person"},
    {"evidence", "Proof or indication"},
    {"suspect", "Person believed to be guilty"},
    {"alibi", "Claim of being elsewhere during a crime"}
};

// Word and hint pairs for the comics theme
const WordHintPair comicsWords[] = {
    {"superhero", "Comics hero"},
    {"villain", "Comics antagonist"},
    {"cape", "Hero's garment"},
    {"power", "Hero's ability"},
    {"mask", "Hero's disguise"}
};

// Function prototypes
void drawHangman(SDL_Renderer* renderer, int wrongGuesses);
void drawWrongGuessImage(SDL_Renderer* renderer, int wrongGuesses, Theme theme);
void drawWord(SDL_Renderer* renderer, const char* word, const char* guessedLetters, int numGuessedLetters);
void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color);
void showMainMenu(SDL_Renderer* renderer, TTF_Font* font);
Theme showThemeMenu(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font);
const WordHintPair* getRandomWordAndHint(Theme theme);
void showCongratulationsScreen(SDL_Renderer* renderer, TTF_Font* font, const char* word);
void showLosingScreen(SDL_Renderer* renderer, TTF_Font* font, const char* word);

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf for text rendering
    if (TTF_Init() < 0) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }

    // Initialize SDL_image for loading PNG images
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Failed to initialize SDL_image: %s\n", IMG_GetError());
        return 1;
    }

    // Create a window for the game
    SDL_Window* window = SDL_CreateWindow("Hangman Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return 1;
    }

    // Create a renderer for drawing on the window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    // Load the font for rendering text
    TTF_Font* font = TTF_OpenFont("OpenSans-Semibold.ttf", 24); // Replace with your font file path
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    // Main game loop
    while (1) {
        // Show main menu and start the game
        showMainMenu(renderer, font);
        Theme selectedTheme = showThemeMenu(window, renderer, font);

        // Get a random word and its hint based on the selected theme
        const WordHintPair* wordHintPair = getRandomWordAndHint(selectedTheme);

        // Initialize variables for tracking guessed letters and wrong guesses
        char guessedLetters[26] = {0};
        int numGuessedLetters = 0;
        int wrongGuesses = 0;
        int gameOver = 0;

        // Flag to indicate whether to redraw the screen
        int needsRedraw = 1;

        // SDL event handling
        SDL_Event e;
        while (!gameOver) {
            // Handle SDL events
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    TTF_CloseFont(font);
                    TTF_Quit();
                    SDL_Quit();
                    return 0;
                } else if (e.type == SDL_KEYDOWN) {
                    char guessedLetter = (char)e.key.keysym.sym;
                    if (guessedLetter >= 'a' && guessedLetter <= 'z') {
                        guessedLetters[numGuessedLetters++] = guessedLetter;

                        // Check if the guessed letter is incorrect
                        if (!strchr(wordHintPair->word, guessedLetter)) {
                            wrongGuesses++;
                        }
                        needsRedraw = 1; // Set flag to redraw the screen
                    }
                }
            }

            // Redraw the screen if needed
            if (needsRedraw) {
                // Clear the renderer
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderClear(renderer);

                // Draw the hangman or wrong guess image
                drawWrongGuessImage(renderer, wrongGuesses, selectedTheme);

                // Draw the word with guessed letters and underscores
                drawWord(renderer, wordHintPair->word, guessedLetters, numGuessedLetters);

                // Draw hint and tries counter
                char hintText[256];
                snprintf(hintText, sizeof(hintText), "Hint: %s", wordHintPair->hint);
                drawText(renderer, font, hintText, 100, 100, (SDL_Color){0, 0, 0});

                char triesText[256];
                snprintf(triesText, sizeof(triesText), "Tries left: %d", 6 - wrongGuesses);
                drawText(renderer, font, triesText, WINDOW_WIDTH - 200, 50, (SDL_Color){0, 0, 0});

                // Update the screen
                SDL_RenderPresent(renderer);

                needsRedraw = 0; // Reset the redraw flag
            }

            // Check game over conditions
            if (wrongGuesses >= 6) {
                // Display losing screen if player runs out of guesses
                showLosingScreen(renderer, font, wordHintPair->word);
                gameOver = 1;
            }

            // Check if player has guessed all letters correctly
            int wordLen = strlen(wordHintPair->word);
            int correctGuesses = 0;
            for (int i = 0; i < wordLen; i++) {
                if (strchr(guessedLetters, wordHintPair->word[i])) {
                    correctGuesses++;
                }
            }
            if (correctGuesses == wordLen) {
                // Display congratulations screen if player guesses the word
                showCongratulationsScreen(renderer, font, wordHintPair->word);
                gameOver = 1;
            }
        }
    }

    // Cleanup resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

// Function to draw the hangman based on the number of wrong guesses
void drawHangman(SDL_Renderer* renderer, int wrongGuesses) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    // Draw the base
    SDL_Rect base = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT - 100, 200, 20};
    SDL_RenderFillRect(renderer, &base);

    // Draw the upright
    SDL_Rect upright = {WINDOW_WIDTH / 2, 100, 20, WINDOW_HEIGHT - 200};
    SDL_RenderFillRect(renderer, &upright);

    // Draw the horizontal beam
    SDL_Rect beam = {WINDOW_WIDTH / 2, 100, 200, 20};
    SDL_RenderFillRect(renderer, &beam);

    // Draw the rope
    SDL_Rect rope = {WINDOW_WIDTH / 2 + 190, 120, 10, 50};
    SDL_RenderFillRect(renderer, &rope);

    // Draw the head
    if (wrongGuesses >= 1) {
        SDL_Rect head = {WINDOW_WIDTH / 2 + 175, 170, 50, 50};
        SDL_RenderFillRect(renderer, &head);
    }

    // Draw the body
    if (wrongGuesses >= 2) {
        SDL_Rect body = {WINDOW_WIDTH / 2 + 190, 220, 20, 100};
        SDL_RenderFillRect(renderer, &body);
    }

    // Draw the left arm
    if (wrongGuesses >= 3) {
        SDL_Rect leftArm = {WINDOW_WIDTH / 2 + 140, 240, 50, 20};
        SDL_RenderFillRect(renderer, &leftArm);
    }

    // Draw the right arm
    if (wrongGuesses >= 4) {
        SDL_Rect rightArm = {WINDOW_WIDTH / 2 + 210, 240, 50, 20};
        SDL_RenderFillRect(renderer, &rightArm);
    }

    // Draw the left leg
    if (wrongGuesses >= 5) {
        SDL_Rect leftLeg = {WINDOW_WIDTH / 2 + 180, 320, 50, 20};
        SDL_RenderFillRect(renderer, &leftLeg);
    }

    // Draw the right leg
    if (wrongGuesses >= 6) {
        SDL_Rect rightLeg = {WINDOW_WIDTH / 2 + 210, 320, 50, 20};
        SDL_RenderFillRect(renderer, &rightLeg);
    }
}

// Function to draw the wrong guess image based on the theme
void drawWrongGuessImage(SDL_Renderer* renderer, int wrongGuesses, Theme theme) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Draw hangman for the pirates theme
    if (theme == PIRATES) {
        drawHangman(renderer, wrongGuesses);
    } else {
        char imagePath[100];
        switch (theme) {
            // Load different images for crime drama and comics themes
            case CRIME_DRAMA:
                snprintf(imagePath, sizeof(imagePath), "a_wrong_guesses_%d.png", wrongGuesses);
                break;
            case COMICS:
                snprintf(imagePath, sizeof(imagePath), "c_wrong_guess_%d.png", wrongGuesses);
                break;
            default:
                return;
        }

        // Load and render image based on the theme
        SDL_Surface* surface = IMG_Load(imagePath);
        if (surface == NULL) {
            printf("Failed to load image: %s\n", SDL_GetError());
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        SDL_Rect rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, texture, NULL, &rect);

        SDL_DestroyTexture(texture);
    }

    // Update the screen
    SDL_RenderPresent(renderer);
}

// Function to draw the word with guessed letters or underscores for unknown letters
void drawWord(SDL_Renderer* renderer, const char* word, const char* guessedLetters, int numGuessedLetters) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    int x = 100;
    int wordLen = strlen(word);
    for (int i = 0; i < wordLen; i++) {
        int guessed = 0;
        for (int j = 0; j < numGuessedLetters; j++) {
            if (word[i] == guessedLetters[j]) {
                guessed = 1;
                break;
            }
        }
        if (guessed) {
            // Draw the letter if guessed correctly
            SDL_Surface* surface;
            SDL_Texture* texture;
            SDL_Rect rect = {x, 50, 30, 50};
            SDL_Color textColor = {0, 0, 0};

            TTF_Font* font = TTF_OpenFont("OpenSans-Semibold.ttf", 24); // Replace with your font file path
            char letter[2] = {word[i], '\0'};
            surface = TTF_RenderText_Solid(font, letter, textColor);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
            TTF_CloseFont(font);
        } else {
            // Draw an underscore for unknown letters
            SDL_Rect underscoreRect = {x, 60, 30, 5};
            SDL_RenderFillRect(renderer, &underscoreRect);
        }
        x += 40;
    }
}

// Function to draw text on the screen
void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Function to display the main menu and start the game
void showMainMenu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Surface* bgSurface = IMG_Load("MenuWallpaper.png");
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect);

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);

    SDL_Event e;
    int startGame = 0;
    while (!startGame) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                startGame = 1;
            }
        }
    }
}

// Function to display the theme selection menu and return the selected theme
Theme showThemeMenu(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Surface* bgSurface = IMG_Load("ThemeWallpaper.png");
    if (!bgSurface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        exit(1);
    }

    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    if (!bgTexture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(bgSurface);
        exit(1);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect);

    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);

    SDL_Event e;
    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                TTF_CloseFont(font);
                TTF_Quit();
                SDL_Quit();
                exit(0);
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_1:
                        return PIRATES;
                    case SDLK_2:
                        return CRIME_DRAMA;
                    case SDLK_3:
                        return COMICS;
                    default:
                        break;
                }
            }
        }
    }
}

// Function to get a random word and hint pair based on the selected theme
const WordHintPair* getRandomWordAndHint(Theme theme) {
    srand((unsigned int)time(NULL));
    int randomIndex;
    switch (theme) {
        case PIRATES:
            randomIndex = rand() % (sizeof(piratesWords) / sizeof(piratesWords[0]));
            return &piratesWords[randomIndex];
        case CRIME_DRAMA:
            randomIndex = rand() % (sizeof(crimeDramaWords) / sizeof(crimeDramaWords[0]));
            return &crimeDramaWords[randomIndex];
        case COMICS:
            randomIndex = rand() % (sizeof(comicsWords) / sizeof(comicsWords[0]));
            return &comicsWords[randomIndex];
        default:
            return NULL;
    }
}

// Function to display the congratulations screen when the player wins
void showCongratulationsScreen(SDL_Renderer* renderer, TTF_Font* font, const char* word) {
    SDL_Surface* bgSurface = IMG_Load("WinScreen.png");
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect);

    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);

    SDL_Color textColor = {255, 255, 255};
    char message[256];
    snprintf(message, sizeof(message), "Congratulations! You guessed the word: %s", word);
    drawText(renderer, font, message, WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 + 200, textColor);

    SDL_RenderPresent(renderer);

    SDL_Event e;
    int done = 0;
    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym  == SDLK_RETURN) {
                done = 1;
            }
        }
    }
}

// Function to display the losing screen when the player runs out of guesses
void showLosingScreen(SDL_Renderer* renderer, TTF_Font* font, const char* word) {
    SDL_Surface* bgSurface = IMG_Load("LoseScreen.png");
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect);

    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);

    SDL_Color textColor = {255, 255, 255};
    char message[256];
    snprintf(message, sizeof(message), "You lost! The word was: %s", word);
    drawText(renderer, font, message, WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 + 200, textColor);

    SDL_RenderPresent(renderer);

    SDL_Event e;
    int done = 0;
    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                done = 1;
            }
        }
    }
}