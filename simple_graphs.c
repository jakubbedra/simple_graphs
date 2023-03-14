#include <Python.h>
#include "structmember.h"
#include <stdio.h>

typedef struct {
    PyObject_HEAD
    short vertices;
    short *edges;
} Graph;

static void Graph_dealloc(Graph *self) {
    free(self->edges);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *Graph_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Graph *self;
    self = (Graph *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->vertices = 0x0000;
        self->edges = NULL;
    }
    return (PyObject *) self;
}

static int Graph_init(Graph *self, PyObject *args, PyObject *kwds) {
    char *txt = "?";
    if (args != NULL) {
        PyArg_ParseTuple(args, "s", &txt);
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

static PyObject *vertices(Graph *self) {
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

static PyObject *number_of_vertices(Graph *self) {
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

static PyObject *vertex_degree(Graph *self, PyObject* args) {
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

// vertex neighbours
// add vertex
// delete vertex

static PyObject* edges(Graph *self) {
    PyObject *edges_set = PySet_New(NULL);
    if (!edges_set) {
        return NULL;
    }

    for (int j = 0; j < 16; j++) {
        short edges = self->edges[j];
        for (int i = 0; i < 16; i++) {
            if ((edges & 0x0001) == 1) {
                PyObject* edge = PyTuple_New(2);
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

static PyObject *number_of_edges(Graph *self) {
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

static PyMemberDef Graph_members[] = {
        {"vertices", T_SHORT, offsetof(Graph, vertices), 0, PyDoc_STR("vertices of the graph")},
        {"edges",    T_SHORT, offsetof(Graph, edges),    0, PyDoc_STR("edges of the graph")},
        {NULL}  /* Sentinel */
};

static PyMethodDef Graph_methods[] = {
        {"number_of_vertices", (PyCFunction) number_of_vertices, METH_NOARGS},
        {"vertices",    (PyCFunction) vertices,    METH_NOARGS},
        {"vertex_degree",    (PyCFunction) vertex_degree,    METH_VARARGS},
        {"number_of_edges",    (PyCFunction) number_of_edges,    METH_NOARGS},
        {"edges",    (PyCFunction) edges,    METH_NOARGS},
        {NULL,                 NULL}
};

static PyTypeObject GraphType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "_simple_graphs.Graph",
        sizeof(Graph),
        0,
        (destructor)Graph_dealloc,                  /* tp_dealloc */
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
        Graph_methods,                              /* tp_methods */
        Graph_members,                              /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        (initproc)Graph_init,                       /* tp_init */
        0,                                          /* tp_alloc */
        Graph_new,                                  /* tp_new */
};

static struct PyModuleDef graphmodule = {
        PyModuleDef_HEAD_INIT,
        "_simple_graphs",
        NULL,
        -1
};

PyMODINIT_FUNC PyInit_simple_graphs(void) {
    PyObject *m;
    if (PyType_Ready(&GraphType) < 0)
        return NULL;

    m = PyModule_Create(&graphmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&GraphType);
    if (PyModule_AddObject(m, "Graph", (PyObject * ) & GraphType) < 0) {
        Py_DECREF(&GraphType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}