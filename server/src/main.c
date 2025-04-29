#include <server.h>

int main()
{
    Game game = {0};
    game.windowWidth = 800;
    game.windowHeight = 600;

    if (!initiate(&game))
        return 1;
    run(&game);
    closeGame(&game);

    return 0;
}
