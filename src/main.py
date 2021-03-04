import tkinter as tk

from random import randint

CANVAS_WIDTH = 161
CANVAS_HEIGHT = 161
CANVAS_BG = "gray75"
CELL_WIDTH = 10
CELL_HEIGHT = 10
CELL_OUTLINE = "gray50"
PIECE_OUTLINE = "black"

COLOR_O = "gold"
COLOR_L = "orange"
COLOR_J = "royal blue"
COLOR_S = "green"
COLOR_Z = "red"
COLOR_I = "deep sky blue"
COLOR_T = "slate blue"


PIECE_COLORS = [
    "deep sky blue",
    "royal blue",
    "orange",
    "gold",
    "green",
    "slate blue",
    "red",
]
PIECE_SHAPES = ["I", "J", "L", "O", "S", "T", "Z"]
SHAPE_TO_COLOR = dict(zip(PIECE_SHAPES, PIECE_COLORS))
SHAPE_TO_VALUE = dict(zip(PIECE_SHAPES, range(1, len(PIECE_SHAPES)+1)))
VALUE_TO_COLOR = {
    0: CANVAS_BG,
    **dict(zip(range(1, len(PIECE_SHAPES)+1), PIECE_COLORS))
}


class Piece:
    def __init__(
        self,
        board,
        x,
        y,
        shape,
        outline=PIECE_OUTLINE,
    ):

        self.x = x
        self.y = y
        self.shape = shape
        self.matrix = None
        self.board = board
        self.cell_width = self.board.cell_width
        self.cell_height = self.board.cell_height
        self.cell_outline = self.board.cell_outline
        self.outline = outline
        self.color = SHAPE_TO_COLOR[shape]
        self.value = SHAPE_TO_VALUE[shape]

    def draw(self):
        m = len(self.matrix)
        j = 0
        for x in range(
            self.x,
            self.x + m * self.cell_width,
            self.cell_width
        ):
            i = 0
            for y in range(
                self.y,
                self.y + m * self.cell_height,
                self.cell_height
            ):
                if self.matrix[i][j] > 0:
                    self.board.create_rectangle(
                        x,
                        y,
                        x + self.cell_width,
                        y + self.cell_height,
                        outline=self.outline,
                        fill=self.color
                    )
                i += 1
            j += 1

    def erase(self):
        m = len(self.matrix)
        j = 0
        for x in range(
            self.x,
            self.x + m * self.cell_width,
            self.cell_width
        ):
            i = 0
            for y in range(
                self.y,
                self.y + m * self.cell_height,
                self.cell_height
            ):
                if self.matrix[i][j] > 0:
                    self.board.create_rectangle(
                        x,
                        y,
                        x + self.cell_width,
                        y + self.cell_height,
                        outline=self.cell_outline,
                        fill=self.board.background
                    )
                i += 1
            j += 1

    def left_border(self):
        m = len(self.matrix)
        border = []
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] > 0:
                    border.append([
                        i + self.y // self.cell_height,
                        j + self.x // self.cell_width
                    ])
                    break
        return border

    def has_left_neighbor(self):
        border = self.left_border()
        for i, j in border:
            if j == 0 or self.board.matrix[i][j-1] > 0:
                return True
        return False

    def right_border(self):
        m = len(self.matrix)
        border = []
        for i in range(m):
            for j in reversed(range(m)):
                if self.matrix[i][j] > 0:
                    border.append([
                        i + self.y // self.cell_height,
                        j + self.x // self.cell_width
                    ])
                    break
        return border

    def has_right_neighbor(self):
        border = self.right_border()
        for i, j in border:
            if (j == self.board.ncols-1) or self.board.matrix[i][j+1] > 0:
                return True
        return False

    def bottom_border(self):
        m = len(self.matrix)
        border = []
        for j in range(m):
            for i in reversed(range(m)):
                if self.matrix[i][j] == 1:
                    border.append([
                        i + self.y // self.cell_height,
                        j + self.x // self.cell_width
                    ])
                    break
        return border

    def has_bottom_neighbor(self):
        border = self.bottom_border()
        for i, j in border:
            if (i == self.board.nrows-1) or self.board.matrix[i+1][j] > 0:
                return True
        return False

    def leftmost_block(self):
        m = len(self.matrix)
        leftmost = m
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] > 0 and j < leftmost:
                    leftmost = j
        return leftmost * self.cell_width + self.x

    def rightmost_block(self):
        m = len(self.matrix)
        rightmost = 0
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] > 0 and j > rightmost:
                    rightmost = j
        return (rightmost + 1) * self.cell_width + self.x

    def bottommost_block(self):
        m = len(self.matrix)
        bottommost = 0
        for i in range(m):
            for j in range(m):
                if self.matrix[i][j] > 0 and i > bottommost:
                    bottommost = i
        return (bottommost + 1) * self.cell_height + self.y

    def move_left(self, event):
        if self.has_left_neighbor():
            return
        self.erase()
        self.x -= self.cell_width
        self.draw()

    def move_right(self, event):
        if self.has_right_neighbor():
            return
        self.erase()
        self.x += self.cell_width
        self.draw()

    def move_down(self):
        if self.has_bottom_neighbor():
            return
        self.erase()
        self.y += self.cell_width
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
            self.x += self.cell_width
        while self.rightmost_block() > self.board.width:
            self.x -= self.cell_width
        while self.bottommost_block() > self.board.width:
            self.y -= self.cell_height
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
            self.x += self.cell_width
        while self.rightmost_block() > self.board.width:
            self.x -= self.cell_width
        while self.bottommost_block() > self.board.height:
            self.y -= self.cell_height
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

        self.winfo_toplevel().title("Tetris")
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

    def draw_board(self, from_row=None, to_row=None):
        if from_row is None:
            from_row = 0
        if to_row is None:
            to_row = self.nrows
        from_pixel = from_row + 1
        to_pixel = to_row * self.cell_height + 1
        for y in range(from_pixel, to_pixel, self.cell_height):
            i = y // self.cell_height
            for x in range(1, self.width, self.cell_width):
                j = x // self.cell_width
                if self.matrix[i][j] > 0:
                    outline = PIECE_OUTLINE
                else:
                    outline = self.cell_outline
                self.create_rectangle(
                    x,
                    y,
                    x + self.cell_width,
                    y + self.cell_height,
                    fill=VALUE_TO_COLOR[self.matrix[i][j]],
                    outline=outline,
                )

        #for x in range(1, self.width, self.cell_width):
        #    for y in range(1, self.height, self.cell_height):
        #        self.create_rectangle(
        #            x,
        #            y,
        #            x + self.cell_width,
        #            y + self.cell_height,
        #            outline=self.cell_outline,
        #        )

    def start(self):
        self.move_piece_down(None)
        self.after(500, self.start)

    def get_random_piece(self):
        pieces = [I, J, L, O, S, T, Z]
        return pieces[randint(0, 5)](self, 1, 1)

    def spawn_piece(self):
        self.piece = self.get_random_piece()
        self.piece.draw()
        self.bind("<Down>", self.move_piece_down)
        self.bind("<Left>", self.piece.move_left)
        self.bind("<Right>", self.piece.move_right)
        self.bind("<d>", self.piece.rotate_clockwise)
        self.bind("<a>", self.piece.rotate_anticlockwise)

    def move_piece_left(self):
        self.piece.move_left()

    def move_piece_right(self):
        self.piece.move_right()

    def move_piece_down(self, event):
        self.piece.move_down()
        if self.piece.has_bottom_neighbor():
            x_offset = self.piece.x // self.cell_height
            y_offset = self.piece.y // self.cell_width
            m = len(self.piece.matrix)
            for i in range(m):
                if i + x_offset > self.height:
                    break
                for j in range(m):
                    if self.piece.matrix[i][j] > 0:
                        self.matrix[y_offset+i][x_offset+j] = self.piece.value
            full_rows = self.get_full_rows()
            if full_rows:
                last_full_row = max(full_rows)
                for row in full_rows:
                    self.shift_matrix_down(row)
                self.draw_board(0, last_full_row+1)
            for row in self.matrix:
                print(row)
            print()
            self.spawn_piece()

    def get_full_rows(self):
        row_indices = []
        y_offset = self.piece.y // self.cell_width
        m = len(self.piece.matrix)
        for i in range(y_offset, min(y_offset + m, self.nrows)):
            full_row = True
            for j in range(self.ncols):
                if self.matrix[i][j] == 0:
                    full_row = False
                    break
            if full_row:
                row_indices.append(i)
        return row_indices

    def shift_matrix_down(self, row_index):
        for i in reversed(range(1, row_index+1)):
            for j in range(self.ncols):
                self.matrix[i][j] = self.matrix[i-1][j]
        self.matrix[0] = [0 for __ in range(self.ncols)]


def main():
    root = tk.Tk()
    board = Board(root)
    board.pack(side=tk.TOP, padx=5, pady=5)
    board.draw_board()
    board.spawn_piece()
    board.start()
    board.mainloop()


if __name__ == "__main__":
    main()