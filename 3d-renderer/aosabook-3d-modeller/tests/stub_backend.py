"""Stub matrix-stack backend standing in for OpenGL in headless tests."""
import numpy as np
from modeller.matrices import IDENTITY


class StubBackend:
    def __init__(self):
        self.stack = [IDENTITY.copy()]
        self.draws = []          # call-list names, in render order
        self.emissions = []      # (call-list name, selected?) per draw
        self.log = []            # ordered render-pass events
        self.projection = None
        self.modelview = None

    def push_matrix(self):
        self.stack.append(self.stack[-1].copy())

    def pop_matrix(self):
        self.stack.pop()

    def mult_matrix(self, matrix):
        self.stack[-1] = self.stack[-1] @ matrix

    def current_matrix(self):
        return self.stack[-1]

    def set_emission(self, selected):
        self._emission = selected

    def draw_call_list(self, name):
        self.draws.append(name)
        self.emissions.append((name, getattr(self, "_emission", False)))

    def set_projection(self, matrix):
        self.projection = matrix

    def set_modelview(self, matrix):
        self.modelview = matrix

    def enable_lighting(self, on):
        self.log.append("lighting_on" if on else "lighting_off")

    def clear(self):
        self.log.append("clear")

    def draw_grid(self):
        self.log.append("grid")

    def flush(self):
        self.log.append("flush")
