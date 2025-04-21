#ifndef SNAKE_CLIENT_H
#define SNAKE_CLIENT_H

int initSnakeClient();
void sendSnakePosition(int x, int y);
void receiveServerUpdate();
void closeSnakeClient();

#endif
