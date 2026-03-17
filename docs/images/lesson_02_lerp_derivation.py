from manim import *

FONT          = "Courier New"
FONT_TIGHT    = "Menlo"
COLOR_BG      = "#1e1e2e"
COLOR_POINT   = "#79c0ff"
COLOR_EDGE    = "#e2e8f0"
COLOR_DX      = "#4f8ef7"
COLOR_SMALL   = "#f47f7f"
COLOR_ITER    = "#3fb950"
COLOR_T       = "#c4b5fd"
COLOR_FORMULA = "#c4b5fd"


def T(s, size=20, color=COLOR_EDGE):
    return Text(s, font=FONT, font_size=size, color=color)


class LerpDerivation(Scene):
    def construct(self):
        self.camera.background_color = COLOR_BG

        A = np.array([-4.0, -1.8, 0])
        B = np.array([ 3.5,  1.8, 0])

        x_steps  = 14
        step_vec = (B - A) / x_steps
        corner   = np.array([B[0], A[1], 0])

        ix0, iy0 = 2,  10
        ix1, iy1 = 16, 17
        idx = ix1 - ix0   # 14
        idy = iy1 - iy0   # 7

        stop_at_ix = 13

        # ── Step 1: endpoints ─────────────────────────────────────────────
        dot_A = Dot(A, radius=0.1, color=COLOR_POINT)
        dot_B = Dot(B, radius=0.1, color=COLOR_POINT)

        lbl_A_sym = T("(x0, y0)", size=20, color=COLOR_POINT)\
                       .next_to(dot_A, LEFT, buff=0.15)
        lbl_A_num = T(f"({ix0}, {iy0})", size=18, color=COLOR_POINT)\
                       .next_to(lbl_A_sym, DOWN, buff=0.08)
        lbl_A_num.set_opacity(0)

        lbl_B_sym = T("(x1, y1)", size=20, color=COLOR_POINT)\
                       .next_to(dot_B, UP, buff=0.35)
        lbl_B_num = T(f"({ix1}, {iy1})", size=18, color=COLOR_POINT)\
                       .next_to(lbl_B_sym, DOWN, buff=0.08)
        lbl_B_num.set_opacity(0)

        self.play(FadeIn(dot_A, dot_B, lbl_A_sym, lbl_B_sym))
        self.add(lbl_A_num, lbl_B_num)
        self.wait(0.8)

        # ── Step 2: line ──────────────────────────────────────────────────
        edge = Line(A, B, color=COLOR_EDGE, stroke_width=2.5)
        self.play(Create(edge))
        self.wait(0.5)

        # ── Step 3: dx and dy — no numbers yet ────────────────────────────
        dx_line = Line(A, corner, color=COLOR_DX, stroke_width=2.0)\
                      .shift(DOWN * 0.55)
        lbl_dx  = T("dx = x1 - x0", size=20, color=COLOR_DX)\
                      .next_to(dx_line, DOWN, buff=0.14)

        dy_line = Line(corner, B, color=COLOR_DX, stroke_width=2.0)\
                      .shift(RIGHT * 0.45)
        lbl_dy  = T("dy = y1 - y0", size=20, color=COLOR_DX)\
                      .next_to(dy_line, RIGHT, buff=0.14)

        self.play(Create(dx_line), FadeIn(lbl_dx))
        self.play(Create(dy_line), FadeIn(lbl_dy))
        self.wait(0.8)

        # ── Condition + symbolic formulas — top-left ──────────────────────
        lbl_cond = T("When dx >= dy  ->  iterate over each x, solving for y",
                     size=20, color=COLOR_EDGE)\
                      .to_corner(UL, buff=0.4)

        lbl_t_sym = T("t = (x - x0) / dx", size=20, color=COLOR_T)\
                       .next_to(lbl_cond, DOWN, buff=0.55)\
                       .align_to(lbl_cond, LEFT)
        lbl_y_sym = T("y = y0 + t * dy", size=20, color=COLOR_FORMULA)\
                       .next_to(lbl_t_sym, DOWN, buff=0.28)\
                       .align_to(lbl_t_sym, LEFT)

        self.play(FadeIn(lbl_cond))
        self.play(FadeIn(lbl_t_sym), FadeIn(lbl_y_sym))
        self.wait(1.0)

        # green summary sentence — positioned manually to avoid align_to on empty text
        summary_y = lbl_y_sym.get_bottom()[1] - 0.45
        summary_x = lbl_y_sym.get_left()[0]
        lbl_summary = T(" ", size=22, color=COLOR_ITER)
        lbl_summary.move_to([summary_x + lbl_summary.width / 2, summary_y, 0])
        self.add(lbl_summary)

        # numeric labels anchored so their left edges align —
        # both start at the right edge of the wider symbolic label
        sym_right_x = max(lbl_t_sym.get_right()[0],
                          lbl_y_sym.get_right()[0]) + 0.35

        def make_num(text, y_ref, color):
            lbl = T(text, size=18, color=color)
            lbl.move_to([sym_right_x + lbl.width / 2,
                         y_ref.get_center()[1], 0])
            return lbl

        lbl_t_num = make_num(" ", lbl_t_sym, COLOR_T)
        lbl_y_num = make_num(" ", lbl_y_sym, COLOR_FORMULA)
        self.add(lbl_t_num, lbl_y_num)

        # ── Step 4: initial iterator (symbolic only) ──────────────────────
        i = 5
        cur      = A + i * step_vec
        cur_base = np.array([cur[0], A[1], 0])

        seg_x = Line(A, cur_base, color=COLOR_SMALL, stroke_width=2.5)
        seg_y = Line(cur_base, cur, color=COLOR_SMALL, stroke_width=2.5)

        lbl_xx = T("x - x0", size=15, color=COLOR_SMALL)\
                     .next_to(seg_x, UP, buff=0.12)
        lbl_yy = T("y - y0", size=15, color=COLOR_SMALL)\
                     .next_to(seg_y, RIGHT, buff=0.12)

        dot_cur  = Dot(cur, radius=0.09, color=COLOR_ITER)
        lbl_xy_dot = T(" ", size=16, color=COLOR_ITER)\
                        .next_to(dot_cur, UP, buff=0.5)

        self.play(Create(seg_x), FadeIn(lbl_xx))
        self.play(Create(seg_y), FadeIn(lbl_yy))
        self.play(FadeIn(dot_cur))
        self.add(lbl_xy_dot)
        self.wait(1.0)

        pixels = VGroup()
        self.add(pixels)

        # ── Step 5: stepping ──────────────────────────────────────────────
        first_step = True

        for step in range(i, stop_at_ix - ix0 + 1):
            nix = ix0 + step
            nt  = (nix - ix0) / idx
            niy = iy0 + nt * idy

            next_pt   = A + step * step_vec
            next_base = np.array([next_pt[0], A[1], 0])

            new_seg_x = Line(A, next_base, color=COLOR_SMALL, stroke_width=2.5)
            new_seg_y = Line(next_base, next_pt, color=COLOR_SMALL, stroke_width=2.5)
            new_dot   = Dot(next_pt, radius=0.09, color=COLOR_ITER)

            # compact red labels — Menlo for tighter character spacing
            new_lbl_xx = Text(f"x-x0 = {nix}-{ix0} = {nix-ix0}",
                              font=FONT_TIGHT, font_size=15, color=COLOR_SMALL)\
                             .next_to(new_seg_x, UP, buff=0.12)
            new_lbl_yy = Text(f"y-y0 = {niy:.1f}-{iy0} = {niy-iy0:.1f}",
                              font=FONT_TIGHT, font_size=15, color=COLOR_SMALL)\
                             .next_to(new_seg_y, RIGHT, buff=0.12)

            new_t_num  = make_num(f"= ({nix}-{ix0})/{idx} = {nt:.2f}",
                                  lbl_t_sym, COLOR_T)
            new_y_num  = make_num(f"= {iy0}+{nt:.2f}*{idy} = {niy:.1f}",
                                  lbl_y_sym, COLOR_FORMULA)

            new_summary = T(f"When x = {nix},  y = {niy:.1f}",
                            size=22, color=COLOR_ITER)
            new_summary.move_to([summary_x + new_summary.width / 2, summary_y, 0])

            new_xy_dot = T(f"({nix}, {niy:.1f})", size=16, color=COLOR_ITER)\
                             .next_to(new_dot, UP, buff=0.5)

            if first_step:
                # animated: segments, dot, t/y numerics, dx/dy labels, endpoint numbers
                new_lbl_dx = T(f"dx = x1 - x0 = {idx}", size=20, color=COLOR_DX)\
                                 .next_to(dx_line, DOWN, buff=0.14)
                new_lbl_dy = T(f"dy = y1 - y0 = {idy}", size=20, color=COLOR_DX)\
                                 .next_to(dy_line, RIGHT, buff=0.14)

                self.play(
                    Transform(seg_x,    new_seg_x),
                    Transform(seg_y,    new_seg_y),
                    Transform(dot_cur,  new_dot),
                    Transform(lbl_t_num, new_t_num),
                    Transform(lbl_y_num, new_y_num),
                    Transform(lbl_summary, new_summary),
                    Transform(lbl_dx,   new_lbl_dx),
                    Transform(lbl_dy,   new_lbl_dy),
                    lbl_A_num.animate.set_opacity(1),
                    lbl_B_num.animate.set_opacity(1),
                    run_time=1.0
                )
                # red labels and green coord: instant swap, no animation
                self.remove(lbl_xx, lbl_yy, lbl_xy_dot)
                lbl_xx, lbl_yy, lbl_xy_dot = new_lbl_xx, new_lbl_yy, new_xy_dot
                self.add(lbl_xx, lbl_yy, lbl_xy_dot)

                first_step = False
                self.wait(5.0)
            else:
                self.play(
                    Transform(seg_x,    new_seg_x),
                    Transform(seg_y,    new_seg_y),
                    Transform(dot_cur,  new_dot),
                    Transform(lbl_t_num, new_t_num),
                    Transform(lbl_y_num, new_y_num),
                    Transform(lbl_summary, new_summary),
                    run_time=5.0
                )
                # instant swap for red labels and green coord
                self.remove(lbl_xx, lbl_yy, lbl_xy_dot)
                lbl_xx, lbl_yy, lbl_xy_dot = new_lbl_xx, new_lbl_yy, new_xy_dot
                self.add(lbl_xx, lbl_yy, lbl_xy_dot)
                self.wait(0.5)

            pixels.add(Dot(next_pt, radius=0.07, color=COLOR_ITER))
            self.add(pixels)

        self.wait(1.2)

        # ── Step 6: end card ──────────────────────────────────────────────
        self.play(
            FadeOut(seg_x, seg_y, dot_cur, pixels, edge,
                    dot_A, dot_B, lbl_A_sym, lbl_A_num,
                    lbl_B_sym, lbl_B_num,
                    dx_line, lbl_dx, dy_line, lbl_dy,
                    lbl_cond, lbl_t_sym, lbl_y_sym,
                    lbl_t_num, lbl_y_num, lbl_summary,
                    lbl_xx, lbl_yy, lbl_xy_dot)
        )

        end1 = T("When dy > dx:", size=28, color=COLOR_EDGE)
        end2 = T("iterate over each y, solving for x the same way",
                 size=24, color=COLOR_T)
        end3 = T("x = x0 + t * dx     where  t = (y-y0) / dy",
                 size=20, color=COLOR_FORMULA)
        VGroup(end1, end2, end3).arrange(DOWN, buff=0.45).move_to(ORIGIN)

        self.play(FadeIn(end1))
        self.play(FadeIn(end2))
        self.play(FadeIn(end3))
        self.wait(2.5)
