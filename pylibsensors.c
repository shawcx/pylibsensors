#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <structmember.h>

#include <sensors/sensors.h>

//=========================================================
// Subfeature Type
//=========================================================

PyObject *pylibsensors_Error;

typedef struct {
    PyObject_HEAD
    sensors_chip_name chip_name;
    int index;
} PyLibSensors_Subfeature;

static int PyLibSensors_Subfeature_Type_init(PyLibSensors_Subfeature *self, PyObject *args, PyObject *kwds) {
    self->index = -1;
    return 0;
}

static void PyLibSensors_Subfeature_Type_dealloc(PyLibSensors_Subfeature *self) {
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject * PyLibSensors_Subfeature_value(PyLibSensors_Subfeature *self, PyObject *ign) {
    int ok;
    double value;
    ok = sensors_get_value(&(self->chip_name), self->index, &value);
    if (0 > ok) {
        Py_RETURN_NONE;
    }
    return PyFloat_FromDouble(value);
}

static PyMethodDef PyLibSensors_Subfeature_methods[] = {
    { "value",  (PyCFunction)PyLibSensors_Subfeature_value,  METH_NOARGS },
    { NULL }
};

PyTypeObject PyLibSensors_Subfeature_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name       = "pylibsensors.Subfeature",
    .tp_basicsize  = sizeof(PyLibSensors_Subfeature),
    .tp_dealloc    = (destructor)PyLibSensors_Subfeature_Type_dealloc,
    .tp_flags      = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods    = PyLibSensors_Subfeature_methods,
    .tp_init       = (initproc)PyLibSensors_Subfeature_Type_init,
    .tp_new        = PyType_GenericNew,
};

//=========================================================
// Chip Type
//=========================================================

typedef struct {
    PyObject_HEAD
    PyObject *features;
    sensors_chip_name chip_name;
} PyLibSensors_Chip;

static int PyLibSensors_Chip_Type_init(PyLibSensors_Chip *self, PyObject *args, PyObject *kwds) {
    self->features = PyDict_New();
    if (self->features == NULL) {
        return -1;
    }
    return 0;
}

static void PyLibSensors_Chip_Type_dealloc(PyLibSensors_Chip *self) {
    Py_DECREF(self->features);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static Py_ssize_t PyLibSensors_Chip_Features_length(PyLibSensors_Chip *self) {
    return PyDict_Size(self->features);
}

static PyObject * PyLibSensors_Chip_Features_subscript(PyLibSensors_Chip *self, PyObject *key) {
    PyObject *value = PyDict_GetItemWithError(self->features, key);
    if (value == NULL) {
        if (!PyErr_Occurred()) {
            PyErr_SetObject(PyExc_KeyError, key);
        }
        return NULL;
    }
    Py_INCREF(value);
    return value;
}

static PyMemberDef PyLibSensors_Chip_members[] = {
    {"features", T_OBJECT_EX, offsetof(PyLibSensors_Chip, features), READONLY},
    {NULL}
};

static PyMappingMethods PyLibSensors_Chip_mapping = {
    .mp_length        = (lenfunc)       PyLibSensors_Chip_Features_length,
    .mp_subscript     = (binaryfunc)    PyLibSensors_Chip_Features_subscript,
    .mp_ass_subscript = NULL,
};

PyTypeObject PyLibSensors_Chip_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name       = "pylibsensors.Chip",
    .tp_basicsize  = sizeof(PyLibSensors_Chip),
    .tp_dealloc    = (destructor)PyLibSensors_Chip_Type_dealloc,
    .tp_as_mapping = &PyLibSensors_Chip_mapping,
    .tp_flags      = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_members    = PyLibSensors_Chip_members,
    .tp_init       = (initproc)PyLibSensors_Chip_Type_init,
    .tp_new        = PyType_GenericNew,
};

//=========================================================
// Module functions
//=========================================================

static PyObject * pylibsensors_chips(PyObject *self, PyObject *args) {
    PyObject *chips;
    PyObject *features;
    PyLibSensors_Chip *_chip;
    PyLibSensors_Subfeature *_subfeature;
    sensors_chip_name chip_match;
    const sensors_chip_name *chip_name;
    const sensors_feature *feature;
    const sensors_subfeature *subfeature;
    const char *match = "*-*";
    char full_name[256];
    int chip_index;
    int feature_idx;
    int subfeature_idx;
    int ok;

    ok = PyArg_ParseTuple(args, "|s", &match);
    if(!ok) {
        return NULL;
    }

    ok = sensors_parse_chip_name(match, &chip_match);

    chips = PyDict_New();
    chip_index = 0;
    for(;;)  {
        chip_name = sensors_get_detected_chips(&chip_match, &chip_index);
        if (!chip_name) {
            break;
        }

        sensors_snprintf_chip_name(full_name, 256, chip_name);

        _chip = (PyLibSensors_Chip *)PyObject_CallObject((PyObject *)&PyLibSensors_Chip_Type, NULL);
        if(NULL == _chip) {
            PyErr_SetString(PyExc_TypeError, "Could not create chip object");
            return NULL;
        }
        PyDict_SetItemString(chips, full_name, (PyObject *)_chip);
        Py_DECREF(_chip);

        feature_idx = 0;
        for(;;) {
            feature = sensors_get_features(chip_name, &feature_idx);
            if (!feature) {
                break;
            }

            features = PyDict_New();
            PyDict_SetItemString(_chip->features, sensors_get_label(chip_name, feature), (PyObject *)features);
            Py_DECREF(features);

            subfeature_idx = 0;
            for(;;) {
                subfeature = sensors_get_all_subfeatures(chip_name, feature, &subfeature_idx);
                if(!subfeature) {
                    break;
                }

                _subfeature = (PyLibSensors_Subfeature *)PyObject_CallObject((PyObject *)&PyLibSensors_Subfeature_Type, NULL);
                if(NULL == _subfeature) {
                    PyErr_SetString(PyExc_TypeError, "Could not create feature object");
                    return NULL;
                }
                memcpy(&_subfeature->chip_name, chip_name, sizeof(sensors_chip_name));
                _subfeature->index = subfeature_idx - 1;
                PyDict_SetItemString(features, subfeature->name, (PyObject *)_subfeature);
                Py_DECREF(_subfeature);
            }
        }
    }

    return chips;
}

//=========================================================
// Module definition
//=========================================================

static PyMethodDef pylibsensors_PyMethodDefs[] = {
    { "chips", pylibsensors_chips, METH_VARARGS },
    { NULL }
};

static PyModuleDef pylibsensors_module = {
    PyModuleDef_HEAD_INIT,
    "pylibsensors",
    NULL,
    -1,
    pylibsensors_PyMethodDefs
};

PyMODINIT_FUNC PyInit_pylibsensors(void) {
    PyObject *module;
    int ok;

    ok = sensors_init(NULL);
    if(ok != 0) {
        return NULL;
    }

    module = PyModule_Create(&pylibsensors_module);
    if(NULL == module) {
        return NULL;
    }

    if(0 > PyType_Ready(&PyLibSensors_Chip_Type)) {
        return NULL;
    }
    Py_INCREF(&PyLibSensors_Chip_Type);

    if(0 > PyType_Ready(&PyLibSensors_Subfeature_Type)) {
        return NULL;
    }
    Py_INCREF(&PyLibSensors_Subfeature_Type);

    pylibsensors_Error = PyErr_NewException("pylibsensors.error", NULL, NULL);
    Py_INCREF(pylibsensors_Error);
    PyModule_AddObject(module, "error", pylibsensors_Error);

    return module;
}
