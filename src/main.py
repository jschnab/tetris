import time
import tkinter as tk

CANVAS_WIDTH = 321
CANVAS_HEIGHT = 321
CANVAS_BG = "gray75"
CELL_WIDTH = 10
CELL_HEIGHT = 10
CELL_OUTLINE = "gray50"
PIECE_OUTLINE = "black"

COLOR_O = "gold"
COLOR_L = "orange"
COLOR_J = "blue"
COLOR_S = "green"
COLOR_Z = "red"
COLOR_I = "deep sky blue"
COLOR_T = "slate blue"


class Piece:
    def __init__(
        self,
        board,
        x,
        y,
        shape,
    ):

        self.x = x
        self.y = y
        self.shape = shape
        self.matrix = None
        self.board = board
        self.cell_width = self.board.cell_width
        self.cell_height = self.board.cell_height
        self.cell_outline = self.board.cell_outline

    def draw(self):
        m = len(self.matrix)
        j = 0
        for x in range(self.x, self.x + m * CELL_WIDTH, CELL_WIDTH):
            i = 0
            for y in range(self.y, self.y + m * CELL_HEIGHT, CELL_HEIGHT):
                if self.matrix[i][j] == 1:
                    self.board.create_rectangle(
                        x,
                        y,
                        x + CELL_WIDTH,
                        y + CELL_HEIGHT,
                        outline=PIECE_OUTLINE,
                        fill=self.color
                    )
                i += 1
            j += 1

    def erase(self):
        m = len(self.matrix)
        j = 0
        for x in range(self.x, self.x + m * CELL_WIDTH, CELL_WIDTH):
            i = 0
            for y in range(self.y, self.y + m * CELL_HEIGHT, CELL_HEIGHT):
                if self.matrix[i][j] == 1:
                    self.board.create_rectangle(
                        x,
                        y,
                        x + CELL_WIDTH,
                        y + CELL_HEIGHT,
                        outline=CELL_OUTLINE,
                        fill=CANVAS_BG
                    )
                i += 1
            j += 1

    def left_border(self):
        m = len(self.matrix)
        border = []
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] == 1:
                    border.append([
                        i + self.y // self.cell_height,
                        j + self.x // self.cell_width
                    ])
                    break
            else:
                border.append([
                    i + self.y // self.cell_height,
                    self.x // self.cell_width
                ])
        return border

    def has_left_neighbor(self):
        border = self.left_border()
        for i, j in border:
            if j == 0 or self.board.matrix[i][j-1] == 1:
                return True
        return False

    def right_border(self):
        m = len(self.matrix)
        border = []
        for i in reversed(range(m)):
            for j in reversed(range(m)):
                if self.matrix[i][j] == 1:
                    border.append([
                        i + self.y // self.cell_height,
                        j + self.x // self.cell_width
                    ])
                    break
            else:
                border.append([
                    i + self.y // self.cell_height,
                    m + self.x // self.cell_width
                ])
        return border

    def has_right_neighbor(self):
        border = self.right_border()
        for i, j in border:
            if (j == self.board.ncols-1) or self.board.matrix[i][j+1] == 1:
                return True
        return False

    def rightmost_block(self):
        m = len(self.matrix)
        rightmost = 0
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] == 1 and j > rightmost:
                    rightmost = j
        return (rightmost + 1) * CELL_WIDTH + self.x

    def bottommost_block(self):
        m = len(self.matrix)
        bottommost = 0
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] == 1 and i > bottommost:
                    bottommost = i
        return (bottommost + 1) * CELL_HEIGHT + self.y

    def move_left(self, event):
        if self.has_left_neighbor():
            return
        self.erase()
        self.x -= CELL_WIDTH
        self.draw()

    def move_right(self, event):
        if self.has_right_neighbor():
            return
        self.erase()
        self.x += CELL_WIDTH
        self.draw()

    def move_down(self, event):
        if self.bottommost_block() >= CANVAS_HEIGHT:
            return
        self.erase()
        self.y += CELL_WIDTH
        self.draw()

    def rotate_clockwise(self, event):
        m = len(self.matrix)
        self.erase()
        new_matrix = [
            [0 for _ in range(m)]
            for __ in range(m)
        ]
        for i in range(m):
            for j in range(m):
                new_matrix[i][j] = self.matrix[m-j-1][i]
        self.matrix = new_matrix
        while self.leftmost_block() < 1:
            self.x += CELL_WIDTH
        while self.rightmost_block() > CANVAS_WIDTH:
            self.x -= CELL_WIDTH
        while self.bottommost_block() > CANVAS_HEIGHT:
            self.y -= CELL_HEIGHT
        self.draw()

    def rotate_anticlockwise(self, event):
        m = len(self.matrix)
        self.erase()
        new_matrix = [
            [0 for _ in range(m)]
            for __ in range(m)
        ]
        for i in range(m):
            for j in range(m):
                new_matrix[i][j] = self.matrix[j][m-i-1]
        self.matrix = new_matrix
        while self.leftmost_block() < 1:
            self.x += CELL_WIDTH
        while self.rightmost_block() > CANVAS_WIDTH:
            self.x -= CELL_WIDTH
        while self.bottommost_block() > CANVAS_HEIGHT:
            self.y -= CELL_HEIGHT
        self.draw()


class O(Piece):
    def __init__(
        self,
        board,
        x,
        y,
        shape="O",
    ):
        super().__init__(board, x, y, "O")
        self.color = COLOR_O
        self.matrix = [
            [1, 1],
            [1, 1]
        ]

    def rotate_clockwise(self, event):
        pass

    def rotate_anticlockwise(self, event):
        pass


class L(Piece):
    def __init__(
        self,
        board,
        x,
        y,
    ):
        super().__init__(board, x, y, "L")
        self.color = COLOR_L
        self.matrix = [
            [0, 1, 0],
            [0, 1, 0],
            [0, 1, 1]
        ]


class J(Piece):
    def __init__(
        self,
        board,
        x,
        y,
    ):
        super().__init__(board, x, y, "J")
        self.color = COLOR_J
        self.matrix = [
            [0, 1, 0],
            [0, 1, 0],
            [1, 1, 0]
        ]


class Z(Piece):
    def __init__(
        self,
        board,
        x,
        y,
    ):
        super().__init__(board, x, y, "J")
        self.color = COLOR_Z
        self.matrix = [
            [0, 0, 0],
            [1, 1, 0],
            [0, 1, 1]
        ]


class S(Piece):
    def __init__(
        self,
        board,
        x,
        y,
    ):
        super().__init__(board, x, y, "S")
        self.color = COLOR_S
        self.matrix = [
            [0, 0, 0],
            [0, 1, 1],
            [1, 1, 0]
        ]


class T(Piece):
    def __init__(
        self,
        board,
        x,
        y,
    ):
        super().__init__(board, x, y, "T")
        self.color = COLOR_T
        self.matrix = [
            [0, 0, 0],
            [1, 1, 1],
            [0, 1, 0]
        ]


class I(Piece):
    def __init__(
        self,
        board,
        x,
        y,
    ):
        super().__init__(board, x, y, "I")
        self.color = COLOR_I
        self.matrix = [
            [0, 0, 1, 0],
            [0, 0, 1, 0],
            [0, 0, 1, 0],
            [0, 0, 1, 0],
        ]


class Board(tk.Canvas):
    def __init__(
        self,
        parent,
        width=CANVAS_WIDTH,
        height=CANVAS_HEIGHT,
        background=CANVAS_BG,
        cell_width=CELL_WIDTH,
        cell_height=CELL_HEIGHT,
        cell_outline=CELL_OUTLINE,
    ):
        super().__init__(
            parent,
            width=width,
            height=height,
            background=background,
        )

        self.focus_set()
        self.width = width
        self.height = height
        self.background = background
        self.cell_width = cell_width
        self.cell_height = cell_height
        self.cell_outline = cell_outline
        self.ncols = width // cell_width
        self.nrows = height // cell_height
        self.matrix = [
            [0 for _ in range(self.ncols)]
            for __ in range(self.nrows)
        ]

        for x in range(1, self.width, self.cell_width):
            for y in range(1, self.height, self.cell_height):
                self.create_rectangle(
                    x,
                    y,
                    x + self.cell_width,
                    y + self.cell_height,
                    outline=self.cell_outline,
                )

    def spawn_piece(self):
        p1 = I(self, 1, 1)
        p1.draw()
        self.bind("<Down>", p1.move_down)
        self.bind("<Left>", p1.move_left)
        self.bind("<Right>", p1.move_right)
        self.bind("<d>", p1.rotate_clockwise)
        self.bind("<a>", p1.rotate_anticlockwise)



def main():
    root = tk.Tk()
    board = Board(root)
    board.pack(side=tk.TOP, padx=5, pady=5)
    board.spawn_piece()
    root.mainloop()


if __name__ == "__main__":
    main()
