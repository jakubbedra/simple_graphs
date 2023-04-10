#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#include <stdlib.h>

#include <Python.h>
#include "structmember.h"
#include <stdio.h>

typedef struct {
    PyObject_HEAD
    short vertices;
    short *edges;
} AdjacencyMatrix;

static void AdjacencyMatrix_dealloc(AdjacencyMatrix *self) {
    free(self->edges);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *AdjacencyMatrix_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    AdjacencyMatrix *self;
    self = (AdjacencyMatrix *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->vertices = 0x0000;
        self->edges = NULL;
    }
    return (PyObject *) self;
}

static int AdjacencyMatrix_init(AdjacencyMatrix *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = {"text", NULL};
    char *txt = "?";

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s", keywords, &txt)) {
        return -1;
    }

    self->edges = malloc(16 * sizeof(short));
    for (int i = 0; i < 16; i++) {
        self->edges[i] = 0x0000;
    }

    int vertices_count = 0;
    for (int i = 0; i < txt[0] - 63; i++) {
        self->vertices = self->vertices << 1;
        self->vertices = self->vertices | 0x0001;
        vertices_count++;
    }

    int k = 0;
    int i = 1;
    int c = 0;

    for (int v = 1; v < vertices_count; v++) {
        for (int u = 0; u < v; u++) {
            if (k == 0) {
                c = txt[i] - 63;
                i++;
                k = 6;
            }
            k--;
            if ((c & (1 << k)) != 0) {
                self->edges[u] = self->edges[u] | (0x0001 << v);
                self->edges[v] = self->edges[v] | (0x0001 << u);
            }
        }
    }

    return 0;
}

static PyObject *vertices(AdjacencyMatrix *self) {
    PyObject *vertices_set = PySet_New(NULL);
    if (!vertices_set) {
        return NULL;
    }

    for (int i = 0; i < 16; i++) {
        if (((self->vertices >> i) & 0x0001) == 1) {
            PyObject *item = PyLong_FromLong(i);
            PySet_Add(vertices_set, item);
            Py_DECREF(item);
        }
    }

    return vertices_set;
}

static PyObject *number_of_vertices(AdjacencyMatrix *self) {
    short ctr = 0;
    short vertices = self->vertices;
    for (int i = 0; i < 16; i++) {
        if ((vertices & 0x0001) == 1) {
            ctr++;
        }
        vertices = vertices >> 1;
    }
    long ret = ctr;
    return PyLong_FromLong(ret);
}

static PyObject *vertex_degree(AdjacencyMatrix *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }

    short ctr = 0;
    short edges = self->edges[v];
    for (int i = 0; i < 16; i++) {
        if ((edges & 0x0001) == 1) {
            ctr++;
        }
        edges = edges >> 1;
    }

    long ret = (long) ctr;
    return PyLong_FromLong(ret);
}

static PyObject *vertex_neighbors(AdjacencyMatrix *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }
    PyObject *neighbors_set = PySet_New(NULL);

    short edges = self->edges[v];
    for (int i = 0; i < 16; i++) {
        if ((edges & 0x0001) == 1) {
            PyObject *item = PyLong_FromLong(i);
            PySet_Add(neighbors_set, item);
            Py_DECREF(item);
        }
        edges = edges >> 1;
    }

    return neighbors_set;
}

static PyObject *add_vertex(AdjacencyMatrix *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }

    short tmp = (0x0001 << v);
    self->vertices = self->vertices | tmp;
    return PyBool_FromLong(1);
}

static PyObject *delete_vertex(AdjacencyMatrix *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }

    self->edges[v] = 0x0000;

    short tmp = 0x0001 << v;
    tmp = ~tmp;

    for (int i = 0; i < 16; i++) {
        self->edges[i] = self->edges[i] & tmp;
    }
    self->vertices = self->vertices & tmp;
    return PyBool_FromLong(1);
}

static PyObject *edges(AdjacencyMatrix *self) {
    PyObject *edges_set = PySet_New(NULL);
    if (!edges_set) {
        return NULL;
    }

    for (int j = 0; j < 16; j++) {
        short edges = self->edges[j];
        for (int i = 0; i < 16; i++) {
            if ((edges & 0x0001) == 1) {
                PyObject *edge = PyTuple_New(2);
                PyTuple_SetItem(edge, 0, PyLong_FromLong(min(i, j)));
                PyTuple_SetItem(edge, 1, PyLong_FromLong(max(i, j)));
                PySet_Add(edges_set, edge);
                Py_DECREF(edge);
            }
            edges = edges >> 1;
        }
    }

    return edges_set;
}

static PyObject *number_of_edges(AdjacencyMatrix *self) {
    short ctr = 0;
    for (int j = 0; j < 16; j++) {
        short edges = self->edges[j];
        for (int i = 0; i < 16; i++) {
            if ((edges & 0x0001) == 1) {
                ctr++;
            }
            edges = edges >> 1;
        }
    }
    long ret = ctr / 2;
    return PyLong_FromLong(ret);
}

static PyObject *is_edge(AdjacencyMatrix *self, PyObject *args) {
    int v, u;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &v, &u);
    }

    short edges_v = self->edges[v];
    edges_v = edges_v >> u;
    edges_v = edges_v & 0x0001;

    return PyBool_FromLong(edges_v);
}

static PyObject *add_edge(AdjacencyMatrix *self, PyObject *args) {
    int v, u;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &v, &u);
    }

    if (v != u) {
        short update_u = (0x0001 << v);
        short update_v = (0x0001 << u);
        self->edges[v] = self->edges[v] | update_v;
        self->edges[u] = self->edges[u] | update_u;
    }
    return PyBool_FromLong(1);
}

static PyObject *delete_edge(AdjacencyMatrix *self, PyObject *args) {
    int v, u;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &v, &u);
    }

    short update_u = ~(0x0001 << v);
    short update_v = ~(0x0001 << u);
    self->edges[v] = self->edges[v] & update_v;
    self->edges[u] = self->edges[u] & update_u;
    return PyBool_FromLong(1);
}

static int color_component(AdjacencyMatrix *self, short vertex, short *colors, short color) {
    int n = getN(self);
    colors[vertex] = color;

    for (int v = 0; v < n; v++) {
        short mask = (0x01 << v);
        if ((mask & self->edges[vertex]) != 0) { // there is an edge
            if (colors[v] == color) {
                return 0;
            }
            if (colors[v] == -1) {
                if (color_component(self, v, colors, (~color) & 0x01) == 0) {
                    return 0;
                }
            }
        }
    }

    return 1;
}

static PyObject *is_bipartite(AdjacencyMatrix *self) {
    if (self->vertices == 0x00) {
        return PyBool_FromLong(1);
    }
    short colors[] =
            {-1, -1, -1, -1, -1, -1, -1, -1,
             -1, -1, -1, -1, -1, -1, -1, -1,};

    int n = getN(self);
    for (int i = 0; i < n; i++) {
        if (colors[i] == -1) {
            if (color_component(self, i, colors, 0x00) == 0) {
                return PyBool_FromLong(0);
            }
        }
    }

    return PyBool_FromLong(1);
}

static int getN(AdjacencyMatrix *self) {
    short n = 0;
    short vertices = self->vertices;
    for (int i = 0; i < 16; i++) {
        if ((vertices & 0x0001) == 1) {
            n++;
        }
        vertices = vertices >> 1;
    }
    return n;
}

static PyMemberDef AdjacencyMatrix_members[] = {
        {"vertices", T_SHORT, offsetof(AdjacencyMatrix, vertices), 0, PyDoc_STR("vertices of the graph")},
        {"edges",    T_SHORT, offsetof(AdjacencyMatrix, edges),    0, PyDoc_STR("edges of the graph")},
        {NULL}
};

static PyMethodDef AdjacencyMatrix_methods[] = {
        {"number_of_vertices", (PyCFunction) number_of_vertices, METH_NOARGS},
        {"vertices",           (PyCFunction) vertices,           METH_NOARGS},
        {"vertex_degree",      (PyCFunction) vertex_degree,      METH_VARARGS},
        {"vertex_neighbors",   (PyCFunction) vertex_neighbors,   METH_VARARGS},
        {"add_vertex",         (PyCFunction) add_vertex,         METH_VARARGS},
        {"delete_vertex",      (PyCFunction) delete_vertex,      METH_VARARGS},
        {"number_of_edges",    (PyCFunction) number_of_edges,    METH_NOARGS},
        {"is_edge",            (PyCFunction) is_edge,            METH_VARARGS},
        {"add_edge",           (PyCFunction) add_edge,           METH_VARARGS},
        {"delete_edge",        (PyCFunction) delete_edge,        METH_VARARGS},
        {"edges",              (PyCFunction) edges,              METH_NOARGS},
        {"is_bipartite",       (PyCFunction) is_bipartite,       METH_NOARGS},
        {NULL,                 NULL}
};

static PyTypeObject AdjacencyMatrixType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "_simple_graphs.AdjacencyMatrix",
        sizeof(AdjacencyMatrix),
        0,
        (destructor)AdjacencyMatrix_dealloc,                  /* tp_dealloc */
        0,                                          /* tp_vectorcall_offset */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_as_async */
        0,                                          /* tp_repr */
        0,                                          /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        0,                                          /* tp_hash */
        0,                                          /* tp_call */
        0,                                          /* tp_str */
        0,                                          /* tp_getattro */
        0,                                          /* tp_setattro */
        0,                                          /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                         /* tp_flags */
        0,                                          /* tp_doc */
        0,                                          /* tp_traverse */
        0,                                          /* tp_clear */
        0,                                          /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        0,                                          /* tp_iter */
        0,                                          /* tp_iternext */
        AdjacencyMatrix_methods,                              /* tp_methods */
        AdjacencyMatrix_members,                              /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        (initproc)AdjacencyMatrix_init,                       /* tp_init */
        0,                                          /* tp_alloc */
        AdjacencyMatrix_new,                                  /* tp_new */
};

static struct PyModuleDef graphmodule = {
        PyModuleDef_HEAD_INIT,
        "_simple_graphs",
        NULL,
        -1
};

PyMODINIT_FUNC PyInit_simple_graphs(void) {
    PyObject *m;
    if (PyType_Ready(&AdjacencyMatrixType) < 0)
        return NULL;

    m = PyModule_Create(&graphmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&AdjacencyMatrixType);
    if (PyModule_AddObject(m, "AdjacencyMatrix", (PyObject * ) & AdjacencyMatrixType) < 0) {
        Py_DECREF(&AdjacencyMatrixType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}