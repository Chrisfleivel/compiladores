/*
 *	Analizador L�xico	
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Pr�ctica de Programaci�n Nro. 1
 *	
 *	Descripcion:
 *	Implementa un analizador l�xico que reconoce n�meros, identificadores, 
 * 	palabras reservadas, operadores y signos de puntuaci�n para un lenguaje
 * 	con sintaxis tipo JSON.
 *	
 */

/*********** Inclusi�n de cabecera **************/
#include "anlex.h"


/************* Variables globales **************/

int consumir;			/* 1 indica al analizador lexico que debe devolver
						el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];		// string utilizado para cargar mensajes de error
token t;				// token global para recibir componentes del Analizador Lexico

// variables para el analizador lexico

FILE *archivo;			// Fuente JSON
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char lexema[TAMLEX];	// Utilizado por el analizador lexico
int delantero=-1;		// Utilizado por el analizador lexico
int fin=0;				// Utilizado por el analizador lexico
int numLinea=1;			// Numero de Linea

/**************** Funciones **********************/


// Rutinas del analizador lexico


void error(const char* mensaje)
{
	printf("Lin %d: Error Lexico. %s.\n",numLinea,mensaje);	
}

void getToken()
{
	int i=0;
	char c=0;
	int acepto=0;
	int estado=0;
	char msg[41];
	entrada e;

	while((c=fgetc(archivo))!=EOF)
	{
		
		if (c==' ' || c=='\t')
			continue;	//eliminar espacios en blanco
		else if(c=='\n')
		{
			//incrementar el numero de linea
			numLinea++;
			continue;
		}	
		else if (c=='"')
		{
			//es un identificador 
			i=0;
			do{
				lexema[i]=c;
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tama�o de buffer");
				
			}while(c!='"');
			lexema[i]=c;
			lexema[i+1]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
			c=0;
			t.pe=buscar(lexema); // palabras reservadas 0 id que ya aparecieron antes
			t.compLex=t.pe->compLex;
			strcpy(t.nombre, "STRING");
			if (t.pe->compLex==-1)
			{
				strcpy(e.lexema,lexema);
				e.compLex=LITERAL_CADENA;
				insertar(e);
				t.pe=buscar(lexema);
				t.compLex=LITERAL_CADENA;
				strcpy(t.nombre, "STRING");
			}
			c=fgetc(archivo);
			break;
		}
		else if (isalpha(c))
		{
			//es un palabra reservada
			i=0;
			do{
				lexema[i]=c;
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tama�o de buffer");
			}while(isalpha(c) || isdigit(c));
			lexema[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
			t.pe=buscar(lexema);
			t.compLex=t.pe->compLex;
			strcpy(t.nombre, lexema);
			if (t.pe->compLex==-1)
			{
				strcpy(e.lexema,lexema);
				e.compLex=BOOL;
				insertar(e);
				t.pe=buscar(lexema);
				t.compLex=BOOL;
				strcpy(t.nombre, lexema);
			}
			break;
		}
		else if (isdigit(c))
		{
				//es un numero
				i=0;
				estado=0;
				acepto=0;
				lexema[i]=c;
				
				while(!acepto)
				{
					switch(estado){
					case 0: //una secuencia netamente de digitos, puede ocurrir . o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=0;
						}
						else if(c=='.'){
							lexema[++i]=c;
							estado=1;
						}
						else if(tolower(c)=='e'){
							lexema[++i]=c;
							estado=3;
						}
						else{
							estado=6;
						}
						break;
					
					case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
						c=fgetc(archivo);						
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=2;
						}
						else if(c=='.')
						{
							i--;
							fseek(archivo,-1,SEEK_CUR);
							estado=6;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 2://la fraccion decimal, pueden seguir los digitos o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=2;
						}
						else if(tolower(c)=='e')
						{
							lexema[++i]=c;
							estado=3;
						}
						else
							estado=6;
						break;
					case 3://una e, puede seguir +, - o una secuencia de digitos
						c=fgetc(archivo);
						if (c=='+' || c=='-')
						{
							lexema[++i]=c;
							estado=4;
						}
						else if(isdigit(c))
						{
							lexema[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 4://necesariamente debe venir por lo menos un digito
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 5://una secuencia de digitos correspondiente al exponente
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=5;
						}
						else{
							estado=6;
						}break;
					case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
						if (c!=EOF)
							ungetc(c,archivo);
						else
							c=0;
						lexema[++i]='\0';
						acepto=1;
						t.pe=buscar(lexema);
						if (t.pe->compLex==-1)
						{
							strcpy(e.lexema,lexema);
							e.compLex=LITERAL_NUM;
							insertar(e);
							t.pe=buscar(lexema);
						}
						t.compLex=LITERAL_NUM;
						strcpy(t.nombre, "NUMBER");
						break;
					case -1:
						if (c==EOF)
							error("No se esperaba el fin de archivo");
						else
							error(msg);
						exit(1);
					}
				}
			break;
		}
		else if (c==':')
		{
			t.compLex=OPASIGNA;
			t.pe=buscar(":");
			strcpy(t.nombre, "DOS_PUNTOS");
			break;
		}
		else if (c==',')
		{
			t.compLex=',';
			t.pe=buscar(",");
			strcpy(t.nombre, "COMA");
			break;
		}
		else if (c=='[')
		{
			t.compLex='[';
			t.pe=buscar("[");
			strcpy(t.nombre, "L_CORCHETE");
			break;
		}
		else if (c==']')
		{
			t.compLex=']';
			t.pe=buscar("]");
			strcpy(t.nombre, "R_CORCHETE");
			break;
		}
		else if (c=='{')
		{
			t.compLex='{';
			t.pe=buscar("{");
			strcpy(t.nombre, "L_LLAVE");
			break;
		}
		else if (c=='}')
		{
			t.compLex='}';
			t.pe=buscar("}");
			strcpy(t.nombre, "R_LLAVE");
			break;
		}		
		else if (c!=EOF)
		{
			sprintf(msg,"%c no esperado",c);
			error(msg);
		}
	}
	if (c==EOF)
	{
		t.compLex=EOF;
		// strcpy(e.lexema,"EOF");
		sprintf(e.lexema,"EOF");
		t.pe=&e;
		strcpy(t.nombre, " ");
	}
	
}

int main(int argc,char* args[])
{
	// inicializar analizador lexico

	initTabla();
	initTablaSimbolos();
	int lineaAnterior = 0;
	if(argc > 1)
	{
		if (!(archivo=fopen(args[1],"rt")))
		{
			printf("Archivo no encontrado.\n");
			exit(1);
		}
		while (t.compLex!=EOF){
			getToken();
			if (lineaAnterior != numLinea){
				printf("\n Lin %d: ",numLinea);
				printf(" \t %s ",t.nombre);
			}
			else{
				printf(" \t %s ",t.nombre);
			}
			lineaAnterior = numLinea;
		}
		fclose(archivo);
	}else{
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}

	return 0;
}
