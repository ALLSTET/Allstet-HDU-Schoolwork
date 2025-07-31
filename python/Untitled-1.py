# FILEPATH: /cloudide/workspace/tic_tac_toe.py
# BEGIN: bks8u2gn22jw
def draw_board(board):
    for row in board:
        print(" | ".join(row))
        print("-" * 9)

def check_win(board, player):
    for row in board:
        if all([cell == player for cell in row]):
            return True

    for col in range(3):
        if all([board[row][col] == player for row in range(3)]):
            return True

    if all([board[i][i] == player for i in range(3)]) or all([board[i][2 - i] == player for i in range(3)]):
        return True

    return False

def is_board_full(board):
    return all(cell in ['X', 'O'] for row in board for cell in row)

def tic_tac_toe():
    board = [[' ' for _ in range(3)] for _ in range(3)]
    current_player = 'X'

    while True:
        draw_board(board)
        row = int(input(f"Player {current_player}, enter row (0-2): "))
        col = int(input(f"Player {current_player}, enter column (0-2): "))

        if board[row][col] == ' ':
            board[row][col] = current_player
        else:
            print("That space is already taken. Try again.")
            continue

        if check_win(board, current_player):
            draw_board(board)
            print(f"Player {current_player} wins!")
            break

        if is_board_full(board):
            draw_board(board)
            print("It's a draw!")
            break

        current_player = 'O' if current_player == 'X' else 'X'

if __name__ == "__main__":
    tic_tac_toe()
# END: bks8u2gn22jw
