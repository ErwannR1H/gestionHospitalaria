#include <iostream>
#include <limits>
#include <string>

using namespace std;

const int TAM_TABLA_HASH = 101;

// Estructuras de datos
struct Paciente {
    string identificador;
    string nombre;
    int edad;
    string centroMedico;
    string diagnostico;
    string tratamiento;
};

Paciente* crearPaciente(string identificador, string nombre, int edad, string centroMedico, string diagnostico, string tratamiento) {
    Paciente* paciente = new Paciente;
    paciente->identificador = identificador;
    paciente->nombre = nombre;
    paciente->edad = edad;
    paciente->centroMedico = centroMedico;
    paciente->diagnostico = diagnostico;
    paciente->tratamiento = tratamiento;
    return paciente;
}

// Colas
struct NodoCola {
    Paciente* paciente;
    NodoCola* siguiente;
};

struct ColaPacientes {
    NodoCola* frente;
    NodoCola* final;
};

struct SistemaTriaje {
    ColaPacientes inmediatos;
    ColaPacientes criticos;
    ColaPacientes moderados;
    ColaPacientes leves;
    int indiceTurno;
    int turnosRestantes;
};

// Hash

struct NodoHash {
    Paciente* paciente;
    NodoHash* siguiente;
};

struct TablaPacientes {
    NodoHash* cubetas[TAM_TABLA_HASH];
};

// Arbol

struct Medicamento {
    string codigo;
    string nombre;
    int existencias;
    string unidad;
};

Medicamento crearMedicamento(string codigo, string nombre, int existencias, string unidad) {
    Medicamento medicamento;
    medicamento.codigo = codigo;
    medicamento.nombre = nombre;
    medicamento.existencias = existencias;
    medicamento.unidad = unidad;
    return medicamento;
}

struct NodoMedicamento {
    Medicamento medicamento;
    NodoMedicamento* izquierdo;
    NodoMedicamento* derecho;
};

// Funciones para las colas y el triaje de pacientes
void inicializarCola(ColaPacientes* cola) {
    cola->frente = NULL;
    cola->final = NULL;
}

// Inicializar todas las colas
void inicializarTriaje(SistemaTriaje* triaje) {
    inicializarCola(&triaje->inmediatos);
    inicializarCola(&triaje->criticos);
    inicializarCola(&triaje->moderados);
    inicializarCola(&triaje->leves);
    triaje->indiceTurno = 0;
    triaje->turnosRestantes = 3;
}

bool estaVacioCola(ColaPacientes* cola) {
    return cola->frente == NULL;
}

void encolar(ColaPacientes* cola, Paciente* paciente) {
    NodoCola* nodo = new NodoCola;
    nodo->paciente = paciente;
    nodo->siguiente = NULL;

    if (cola->final == NULL) {
        // Si la cola esta vacia, el nuevo nodo es el primero y el ultimo
        cola->frente = nodo;
        cola->final = nodo;
    } else {
        // Sino, tenemos que agregar el nodo al final
        cola->final->siguiente = nodo;
        cola->final = nodo;
    }
}

// eliminar el primer elemento de la lista
Paciente * desencolar(ColaPacientes* cola) {
    if (cola->frente == NULL) { 
        return NULL;
    }

    NodoCola* antiguoFrente = cola->frente;
    Paciente * paciente = antiguoFrente->paciente;
    cola->frente = antiguoFrente->siguiente;
    if (cola->frente == NULL) {
        cola->final = NULL;
    }

    delete antiguoFrente;
    return paciente;
}

void imprimirCola(ColaPacientes* cola) {
    NodoCola* actual = cola->frente;
    if (actual == NULL) {
        cout << "(sin pacientes)" << endl;
        return;
    }

    while (actual != NULL) {
        cout << actual->paciente->nombre << " | DNI: " << actual->paciente->identificador << " | " << actual->paciente->diagnostico << endl;
        actual = actual->siguiente;
    }
}

// Agregar un paciente a la cola prioridad
void agregarAlTriaje(SistemaTriaje* triaje, Paciente* paciente, int prioridad) {

    switch (prioridad) {
        case 1:
            encolar(&triaje->inmediatos, paciente);
            break;
        case 2:
            encolar(&triaje->criticos, paciente);
            break;
        case 3:
            encolar(&triaje->moderados, paciente);
            break;
        case 4:
            encolar(&triaje->leves, paciente);
            break;
        default:
            break;
    }
}

// Permite de saber si un paciente esta en una cola especifica
bool pacienteEnCola(ColaPacientes* cola, string identificador) {
    NodoCola* actual = cola->frente;
    while (actual != NULL) {
        if (actual->paciente->identificador == identificador) {
            return true;
        }
        actual = actual->siguiente;
    }

    return false;
}

// Permite de saber si un paciente esta en alguna cola
int pacienteEnEspera(SistemaTriaje* triaje, string identificador) {
    return pacienteEnCola(&triaje->inmediatos, identificador) ||
           pacienteEnCola(&triaje->criticos, identificador) ||
           pacienteEnCola(&triaje->moderados, identificador) ||
           pacienteEnCola(&triaje->leves, identificador);
}

// Permite de obtener el numbro de turno por un indice de turno (= una prioridad)
int obtenerCuotaTurno(int indiceTurno) {
    if (indiceTurno == 0) {
        // Criticos
        return 3;
    }
    if (indiceTurno == 1) {
        // Moderados
        return 2;
    }
    // Leve
    return 1;
}

// Saber quien es el proximo paciente
// Con esta estrategia :
//   Si hay uno en inmediato, es el proximo
//   Sino : 
//         Nos llevamos 3 en los críticos (si no hay inmediato)
//         Y después llevamos 2 en los moderados (si no hay inmediato)
//         Y uno en leves (si no hay inmediato)
//         Y despuès 3 criticos, 2 moderados, 1 leve ...
// De esta manera, no habrá problemas de hambruna en ninguna cola
Paciente * atenderSiguientePaciente(SistemaTriaje* triaje) {
    Paciente * paciente = desencolar(&triaje->inmediatos);
    if (paciente != NULL) {
        return paciente;
    }

    // El orden : 2 por criticos, 3 por moderados y 4 por leve
    for (int intentos = 0; intentos < 3; intentos++) {
        ColaPacientes* colaActual = NULL;

        if (triaje->indiceTurno == 0) {
            colaActual = &triaje->criticos;
        } else if (triaje->indiceTurno == 1) {
            colaActual = &triaje->moderados;
        } else {
            colaActual = &triaje->leves;
        }

        // Si la cola esta vacia, entonces tenemos que usar la siguiente prioridad
        if (estaVacioCola(colaActual)) {
            triaje->indiceTurno = (triaje->indiceTurno + 1) % 3;
            triaje->turnosRestantes = obtenerCuotaTurno(triaje->indiceTurno);
        } else {
            
            // La cola no esta vacia, entonces hay un paciente que podemos desencolar
            paciente = desencolar(colaActual);
            triaje->turnosRestantes--;
            if (triaje->turnosRestantes == 0) {
                // Proxima prioridad
                triaje->indiceTurno = (triaje->indiceTurno + 1) % 3;
                triaje->turnosRestantes = obtenerCuotaTurno(triaje->indiceTurno);
            }
            return paciente;
        }
    }

    return NULL;
}

// Imprimir el contenido de todas las colas
void imprimirTriaje(SistemaTriaje* triaje) {
    cout << "=== PACIENTES DE ATENCIÓN INMEDIATA ===" << endl;
    imprimirCola(&triaje->inmediatos);
    cout << "=== PACIENTES CRÍTICOS ===" << endl;
    imprimirCola(&triaje->criticos);
    cout << "=== PACIENTES MODERADOS ===" << endl;
    imprimirCola(&triaje->moderados);
    cout << "=== PACIENTES LEVES ===" << endl;
    imprimirCola(&triaje->leves);
}

// Permite de vaciar une cola
void vaciarCola(ColaPacientes* cola) {
    while (!estaVacioCola(cola)) {
        desencolar(cola);
    }
}

// ====================== tabla de hash de pacientes ====================== 
void inicializarTablaPacientes(TablaPacientes* tabla) {
    for (int i = 0; i < TAM_TABLA_HASH; i++) {
        tabla->cubetas[i] = NULL;
    }
}

// Vamos a sumar los caracteres ASCII modulo el tamano de la tabla
int calcularIndiceHash(string identificador) {
    int suma = 0;
    for (int i = 0; i < identificador.length(); i++) {
        suma = (suma + identificador[i]);
    }
    return suma % TAM_TABLA_HASH;
}

// Buscar un paciente con su identificador. Return un puntero del paciente si existe, NULL sino
Paciente* buscarPaciente(TablaPacientes* tabla, string identificador) {
    int indice = calcularIndiceHash(identificador);
    NodoHash* actual = tabla->cubetas[indice];

    // Vamos a verificar cado nodo del indice hash, para saber si hay el paciente o no
    while (actual != NULL) {
        if (actual->paciente->identificador == identificador) {
            return actual->paciente;
        }
        actual = actual->siguiente;
    }

    return NULL;
}

// Agregar el paciente en parametro a la tabla de hash
Paciente* guardarPaciente(TablaPacientes* tabla, Paciente* paciente) {
    Paciente* existente = buscarPaciente(tabla, paciente->identificador);
    if (existente != NULL) {
        // Si hay un nuevo puntero del paciente (con el mismo identificador), tal vez con nuevos datos
        // Vamos a actualizar el paciente
        if (existente != paciente) {
            *existente = *paciente;
            delete paciente;
        }
        return existente;
    }

    // El paciente no existe todavia
    int indice = calcularIndiceHash(paciente->identificador);
    NodoHash* nodo = new NodoHash;
    nodo->paciente = paciente;

    // Las colisiones se almacenan en una lista enlazada en cada cubeta
    // Este nodo es el nuevo primero de la lista
    nodo->siguiente = tabla->cubetas[indice];
    tabla->cubetas[indice] = nodo;
    return paciente;
}

void imprimirTodosLosPacientes(TablaPacientes* tabla) {
    bool encontrado = false;
    for (int i = 0; i < TAM_TABLA_HASH; i++) {
        NodoHash* actual = tabla->cubetas[i];
        while (actual != NULL) {
            Paciente* paciente = actual->paciente;
            cout << "DNI: " << paciente->identificador << " | " << paciente->nombre << " | Edad: " << paciente->edad << " | Centro: " << paciente->centroMedico << endl 
                << "  Diagnóstico: " << paciente->diagnostico << endl 
                << "  Tratamiento: " << paciente->tratamiento << endl;
            encontrado = true;
            actual = actual->siguiente;
        }
    }
    if (!encontrado) {
        cout << "No hay historiales registrados." << endl;
    }
}

void liberarTablaPacientes(TablaPacientes* tabla) {
    for (int i = 0; i < TAM_TABLA_HASH; i++) {
        NodoHash* actual = tabla->cubetas[i];
        while (actual != NULL) {
            NodoHash* nodoAntiguo = actual;
            actual = actual->siguiente;
            delete nodoAntiguo->paciente;
            delete nodoAntiguo;
        }
        tabla->cubetas[i] = NULL;
    }
}

// ============= arbol binario de búsqueda del inventario ============= 

// Insertar a la izquierda si el codigo es menor, a la derecha sino
NodoMedicamento* insertarMedicamento(NodoMedicamento* raiz, Medicamento medicamento) {
    if (raiz == NULL) {
        NodoMedicamento* nodo = new NodoMedicamento;
        nodo->medicamento = medicamento;
        nodo->izquierdo = NULL;
        nodo->derecho = NULL;
        return nodo;
    }

    if (medicamento.codigo < raiz->medicamento.codigo) {
        raiz->izquierdo = insertarMedicamento(raiz->izquierdo, medicamento);
    } else if (medicamento.codigo > raiz->medicamento.codigo) {
        raiz->derecho = insertarMedicamento(raiz->derecho, medicamento);
    } else {
        // igual entonces tenemos que reemplaz (si hay una actualizacion o algo como asi...)
        raiz->medicamento = medicamento;
    }

    return raiz;
}

// Buscar con la misma logica : a la izquierda si el codigo es menor, a la derecha sino
Medicamento* buscarMedicamento(NodoMedicamento* raiz, string codigo) {
    NodoMedicamento* actual = raiz;
    while (actual != NULL && actual->medicamento.codigo != codigo) {
        if (codigo < actual->medicamento.codigo) {
            actual = actual->izquierdo;
        } else {
            actual = actual->derecho;
        }
    }

    // aqui dos opciones : o bien lo hemos encontrado, o bien hemos llegado hasta el fondo, a un NULL
    if (actual == NULL) {
        return NULL;
    }

    return &actual->medicamento;
}

// Vamos a utilizar un recorrido infijo (imprimir la izquierda, el nodo actual y después a la derecha)
// De esta manera, los codigos van a estar ordenado de manera creciente
void imprimirInventarioEnOrden(NodoMedicamento* raiz) {
    if (raiz == NULL) {
        return;
    }

    imprimirInventarioEnOrden(raiz->izquierdo);
    cout << raiz->medicamento.codigo << " | " << raiz->medicamento.nombre << " | Existencias: " << raiz->medicamento.existencias << ' ' << raiz->medicamento.unidad << endl;
    imprimirInventarioEnOrden(raiz->derecho);
}

// Liberar todos los hijos y después el nodo actual
void liberarInventario(NodoMedicamento* raiz) {
    // Caso de base
    if (raiz == NULL) {
        return;
    }

    liberarInventario(raiz->izquierdo);
    liberarInventario(raiz->derecho);
    delete raiz;
}

// ========= Funciones de entrada =========
string leerTexto(string mensaje) {
    cout << mensaje;
    string valor;
    getline(cin, valor);
    return valor;
}

int leerEntero(string mensaje, int minimo, int maximo) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor && valor >= minimo && valor <= maximo) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cout << "Entrada no válida. Ingrese un número entre "
             << minimo << " y " << maximo << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// ========= datos de prueba =========

void cargarDatosDePrueba(SistemaTriaje* triaje, TablaPacientes* pacientes, NodoMedicamento** inventario) {
    guardarPaciente(pacientes, crearPaciente("10010001", "Ana Torres", 34, "Hospital Central", "Asma", "Broncodilatador y control"));
    guardarPaciente(pacientes, crearPaciente("10010002", "Luis Ramírez", 58, "Clínica Norte", "Hipertensión", "Control de presión"));
    guardarPaciente(pacientes, crearPaciente("10010003", "María Salazar", 27, "Clínica Sur", "Fractura de radio", "Inmovilización y seguimiento"));
    guardarPaciente(pacientes, crearPaciente("10010004", "José Paredes", 71, "Hospital Central", "Diabetes tipo 2", "Control metabólico"));

    Paciente* pacienteTriaje = crearPaciente("P-001", "Carlos Vega", 45, "Hospital Central", "Dolor torácico", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);
    pacienteTriaje = crearPaciente("P-013", "Isabel Herrera", 67, "Hospital Central", "Paro cardiorrespiratorio", "Atención inmediata");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 1);
    pacienteTriaje = crearPaciente("P-004", "Rosa Méndez", 52, "Clínica Norte", "Hemorragia", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);
    pacienteTriaje = crearPaciente("P-005", "Mario Ríos", 39, "Clínica Sur", "Reacción alérgica grave", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);
    pacienteTriaje = crearPaciente("P-006", "Lucía Castro", 63, "Hospital Central", "Dificultad respiratoria", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);

    pacienteTriaje = crearPaciente("P-002", "Elena Rojas", 30, "Clínica Norte", "Fiebre alta", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 3);
    pacienteTriaje = crearPaciente("P-007", "Andrés León", 46, "Clínica Este", "Dolor abdominal", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);
    pacienteTriaje = crearPaciente("P-008", "Carmen Díaz", 55, "Clínica Oeste", "Fractura de pierna", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);
    pacienteTriaje = crearPaciente("P-009", "Jorge Silva", 33, "Hospital Central", "Herida profunda", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 2);

    pacienteTriaje = crearPaciente("P-003", "Pedro Luna", 21, "Clínica Sur", "Herida leve", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 4);
    pacienteTriaje = crearPaciente("P-010", "Paola Núñez", 24, "Clínica Norte", "Dolor de garganta", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 3);
    pacienteTriaje = crearPaciente("P-011", "Sergio Ponce", 29, "Clínica Este", "Migraña leve", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 3);
    pacienteTriaje = crearPaciente("P-012", "Elisa Fuentes", 18, "Clínica Oeste", "Contusión leve", "Pendiente");
    pacienteTriaje = guardarPaciente(pacientes, pacienteTriaje);
    agregarAlTriaje(triaje, pacienteTriaje, 3);

    // Pacientes de prueba para comprobar colisiones en la tabla hash.
    guardarPaciente(pacientes, crearPaciente("TEST-AB", "Paciente Prueba A", 28,
        "Hospital Central", "Prueba de colisión A", "Observación"));
    guardarPaciente(pacientes, crearPaciente("TEST-BA", "Paciente Prueba B", 31,
        "Clínica Norte", "Prueba de colisión B", "Observación"));

    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-001", "Paracetamol 500 mg", 250, "tabletas"));
    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-002", "Amoxicilina 500 mg", 120, "cápsulas"));
    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-003", "Suero fisiológico", 80, "bolsas"));
    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-004", "Ibuprofeno 400 mg", 160, "tabletas"));
    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-007", "Guantes descartables", 500, "unidades"));
    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-050", "Alcohol medicinal", 75, "frascos"));
    *inventario = insertarMedicamento(*inventario, crearMedicamento("MED-060", "Gasas esterilizadas", 300, "paquetes"));

}

// ====================== MENUS ====================== 

void menuTriaje(SistemaTriaje* triaje, TablaPacientes* pacientes) {
    cout << endl << "--- TRIAJE DE URGENCIAS ---" << endl
         << "1. Registrar paciente" << endl
         << "2. Atender al siguiente paciente" << endl
         << "3. Ver las colas de espera" << endl
         << "0. Volver al menú principal" << endl;
    int opcion = leerEntero("Opción: ", 0, 3);

    if (opcion == 1) {
        cout << endl << "1. Crear un paciente nuevo" << endl
             << "2. Agregar a la cola un paciente ya registrado" << endl
             << "0. Volver al menú de triaje" << endl;
        int tipoPaciente = leerEntero("Opción: ", 0, 2);
        if (tipoPaciente == 0) return;

        Paciente* paciente = NULL;
        if (tipoPaciente == 1) {
            string identificador = leerTexto("Identificador nuevo: ");
            if (buscarPaciente(pacientes, identificador) != NULL) {
                cout << "Error: ya existe un paciente con ese identificador. No se agregó a la cola." << endl;
                return;
            }

            string nombre = leerTexto("Nombre: ");
            int edad = leerEntero("Edad: ", 0, 120);
            string centroMedico = leerTexto("Centro médico: ");
            string diagnostico = leerTexto("Motivo/diagnóstico inicial: ");
            paciente = crearPaciente(identificador, nombre, edad, centroMedico, diagnostico, "Pendiente");
        } else {
            string identificador = leerTexto("Identificador del paciente registrado: ");
            paciente = buscarPaciente(pacientes, identificador);
            if (paciente == NULL) {
                cout << "Error: no existe un paciente registrado con ese identificador. No se agregó a la cola." << endl;
                return;
            }
        }

        // Si el paciente esta todavia en una cola
        if (pacienteEnEspera(triaje, paciente->identificador)) {
            // Si es una creacion, el paciente ya existe. Entonces tenemos que delete el ptr
            if (tipoPaciente == 1) {
                delete paciente;
            }
            cout << "Error: el paciente ya está en una cola de espera." << endl;
            return;
        }

        cout << "Prioridad: 1) Inmediato  2) Crítico  3) Moderado  4) Leve" << endl;
        cout << "0) Cancelar y volver" << endl;
        int prioridad = leerEntero("Nivel: ", 0, 4);
        if (prioridad == 0) {
            if (tipoPaciente == 1) {
                delete paciente;
            }
            return;
        }

        if (tipoPaciente == 1) {
            // es un nuevo paciente, tenemos que guardar en la tabla de hash
            paciente = guardarPaciente(pacientes, paciente);
        }
        agregarAlTriaje(triaje, paciente, prioridad);
        cout << "Paciente agregado a la cola de triaje." << endl;
    } else if (opcion == 2) {
        Paciente* paciente = atenderSiguientePaciente(triaje);

        if (paciente == NULL) {
            cout << "No hay pacientes pendientes." << endl;
        } else {
            cout << "Atendiendo : "
                 << paciente->nombre << " | DNI: " << paciente->identificador
                 << " | " << paciente->diagnostico << endl;
        }
    } else if (opcion == 3) {
        imprimirTriaje(triaje);
    }
}

void menuPacientes(TablaPacientes* pacientes) {
    cout << "\n--- HISTORIALES CLÍNICOS (TABLA HASH) ---\n"
         << "1. Buscar por identificador\n"
         << "2. Registrar/actualizar historial\n"
         << "3. Mostrar todos los historiales\n"
         << "0. Volver al menú principal\n";
    int opcion = leerEntero("Opción: ", 0, 3);

    if (opcion == 1) {
        string identificador = leerTexto("Identificador: ");
        Paciente* paciente = buscarPaciente(pacientes, identificador);
        if (paciente == NULL) {
            cout << "No se encontró el historial.\n";
        } else {
            cout << "DNI: " << paciente->identificador << " | "
                 << paciente->nombre << " | Edad: " << paciente->edad
                 << " | Centro: " << paciente->centroMedico
                 << "\nDiagnóstico: " << paciente->diagnostico
                 << "\nTratamiento: " << paciente->tratamiento << '\n';
        }
    } else if (opcion == 2) {
        string identificador = leerTexto("Identificador único: ");
        string nombre = leerTexto("Nombre ficticio: ");
        int edad = leerEntero("Edad: ", 0, 120);
        string centroMedico = leerTexto("Centro médico: ");
        string diagnostico = leerTexto("Diagnóstico: ");
        string tratamiento = leerTexto("Tratamiento: ");
        guardarPaciente(pacientes, crearPaciente(identificador, nombre, edad,
            centroMedico, diagnostico, tratamiento));
        cout << "Historial registrado/actualizado en la tabla hash.\n";
    } else if (opcion == 3) {
        imprimirTodosLosPacientes(pacientes);
    }
}

void menuInventario(NodoMedicamento** inventario) {
    cout << endl << "--- INVENTARIO DE FARMACIA ---" << endl
         << "1. Buscar medicamento" << endl
         << "2. Registrar/actualizar medicamento" << endl
         << "3. Registrar entrada o salida de existencias" << endl
         << "4. Reporte ordenado del inventario" << endl
         << "0. Volver al menú principal" << endl;
    int opcion = leerEntero("Opción: ", 0, 4);

    if (opcion == 1) {
        string codigo = leerTexto("Código: ");
        Medicamento* medicamento = buscarMedicamento(*inventario, codigo);
        if (medicamento == NULL) {
            cout << "Medicamento no encontrado." << endl;
        } else {
            cout << medicamento->codigo << " | " << medicamento->nombre << " | Existencias: " << medicamento->existencias << ' ' << medicamento->unidad << endl;
        }
    } else if (opcion == 2) {
        string codigo = leerTexto("Código único: ");
        string nombre = leerTexto("Nombre: ");
        int existencias = leerEntero("Existencias iniciales: ", 0, 1000000000);
        string unidad = leerTexto("Unidad (tabletas, frascos, etc.): ");
        *inventario = insertarMedicamento(*inventario, crearMedicamento(codigo, nombre, existencias, unidad));
        cout << "Medicamento registrado/actualizado." << endl;
    } else if (opcion == 3) {
        string codigo = leerTexto("Código del medicamento: ");
        Medicamento* medicamento = buscarMedicamento(*inventario, codigo);
        if (medicamento == NULL) {
            cout << "Medicamento no encontrado." << endl;
            return;
        }

        cout << "Existencias actuales: " << medicamento->existencias << ' ' << medicamento->unidad << endl;
        cout << "1. Entrada  2. Salida  0. Volver al menú de inventario" << endl;
        int movimiento = leerEntero("Movimiento: ", 0, 2);
        if (movimiento == 0) {
            return;
        }
        int cantidad = leerEntero("Cantidad: ", 1, 1000000000);
        if (movimiento == 1) {
            if (cantidad > 2147483647 - medicamento->existencias) {
                cout << "La cantidad supera el límite permitido." << endl;
                return;
            }
            medicamento->existencias += cantidad;
        } else if (movimiento == 2) {
            if (cantidad > medicamento->existencias) {
                cout << "Existencias insuficientes; no se registró la salida." << endl;
                return;
            }
            medicamento->existencias -= cantidad;
        }
        cout << "Movimiento aplicado. Nuevas existencias: " << medicamento->existencias << ' ' << medicamento->unidad << endl;
    } else {
        if (*inventario == NULL) {
            cout << "El inventario está vacío." << endl;
        } else {
            cout << endl << "=== REPORTE DE INVENTARIO (ORDENADO POR CÓDIGO) ===" << endl;
            imprimirInventarioEnOrden(*inventario);
        }
    }
}

// ====================== MAIN ====================== 
int main() {
    SistemaTriaje triaje;
    TablaPacientes pacientes;
    NodoMedicamento* inventario = NULL;

    inicializarTriaje(&triaje);
    inicializarTablaPacientes(&pacientes);
    cargarDatosDePrueba(&triaje, &pacientes, &inventario);

    cout << "=============================================\n"
         << " MEDICARE CONNECT - SISTEMA DE GESTIÓN LOCAL\n"
         << " Datos de demostración ficticios precargados\n"
         << "=============================================\n";

    bool ejecutar = true;
    while (ejecutar) {
        cout << "\nMENÚ PRINCIPAL\n"
             << "1. Urgencias y triaje\n"
             << "2. Historiales clínicos\n"
             << "3. Inventario de farmacia\n"
             << "0. Salir\n";

        int opcion = leerEntero("Seleccione una opción: ", 0, 3);

        switch (opcion) {
            case 0:
                ejecutar = false;
                cout << "Sistema finalizado. Gracias.\n";
                break;
            case 1:
                menuTriaje(&triaje, &pacientes);
                break;
            case 2:
                menuPacientes(&pacientes);
                break;
            case 3:
                menuInventario(&inventario);
                break;
        }
    }

    // Tenemos que vaciar todo
    vaciarCola(&triaje.criticos);
    vaciarCola(&triaje.moderados);
    vaciarCola(&triaje.leves);
    vaciarCola(&triaje.inmediatos);
    liberarTablaPacientes(&pacientes);
    liberarInventario(inventario);
    return 0;
}
