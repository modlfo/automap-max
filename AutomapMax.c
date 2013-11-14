
/**
 * Automap-Max: An Max object to create an simple Automap server
 *
 * ##copyright##
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 * 
 * @author              Leonardo Laguna Ruiz
 * @modified            February 26, 2010
 * @version             1.0
 *
 * Based on the Automap example provided by Novation
 */

#include "ext.h"                            // standard Max include, always required
#include "ext_obex.h"                       // required for new style Max object

#include "include/Automap_c.h"              // Automap    

#define MAX_PARAM_NUMBER 512

////////////////////////// object struct
typedef struct _automap_max 
{
    t_object                    ob;         // the object itself (must be first)
	void* m_outlet;
} t_automap_max;

////////////////////////// Automap variables

static int opened=0;

ConnectionHandle *  m_Connection = NULL;
ClientHandle        m_This;
EditorHandle        m_Editor;

char        clientName  [AUTOMAP_NAME_LENGTH]   = "Max Client";
char        manufacturer[AUTOMAP_NAME_LENGTH]   = "None";
char        clientType  [AUTOMAP_NAME_LENGTH]   = "Example";
AutomapGUID m_GUID                              = {0x6ddbcc94,0xca6a74ec,0x0b4493f7,0x10bec5b8};
char        saveName    [AUTOMAP_NAME_LENGTH]   = "ClientName";
char        instanceName[AUTOMAP_NAME_LENGTH]   = "Max Client";

int              nParamInfo = 0;        // Number of initialized parameters   
AutomapParamInfo paramInfoArray [MAX_PARAM_NUMBER];   // Allocating this with a fixed size. I don't think I'm going to use them all (for now)
float            paramValues    [MAX_PARAM_NUMBER];
int              update         [MAX_PARAM_NUMBER];


/////////////////////////////////////////////////////////////////////
// Editor implementation
/////////////////////////////////////////////////////////////////////
void    MyEditor_OnLearnChanged()
{
    post("Learn: ");
    if      (Connection_LearnOnce(m_Connection)){post("Learn: Once\n");}
    else if (Connection_Learn(m_Connection))    {post("Learn: On\n");}
    else                                        {post("Learn: Off\n");}
}
void    MyEditor_OnAutomapChanged()
{
    if (Connection_Automap(m_Connection))       {post("Automap: Active\n");}
    else                                        {post("Automap: Inactive\n");}
}
void    MyEditor_OnClearChanged()
{
    if (Connection_ClearOnce(m_Connection))     {post("Clear: Once\n");}
    else if (Connection_Clear(m_Connection))    {post("Clear: On\n");}
    else                                        {post("Clear: Off\n");}
}

///////////////////////// Automap client implementation

void        MyClient_GetName(char *name)
{
    strcpy(name, clientName);
}
void        MyClient_GetMfr(char *mfr)
{
    strcpy(mfr, manufacturer);
}
void        MyClient_GetType(char *type)
{
    strcpy(type, clientType);
}
AutomapGUID MyClient_GetGUID()
{
    return m_GUID;
}
void        MyClient_GetSaveName(char *name)
{
    strcpy(name, saveName);
}

void        MyClient_GetInstanceName(char *name)
{
    strcpy(name, instanceName);
}
void        MyClient_SaveInstanceName(const char *name)
{
    strcpy(instanceName, name);   
}

bool        MyClient_GetParamInfo(int idx, AutomapParamInfo* data)
{
	AutomapParamInfo *selected;
	if(idx<nParamInfo){
		selected = paramInfoArray+idx;
		strcpy(data->name, selected->name);
		data->maxInteger = selected->maxInteger;
		data->minInteger = selected->minInteger;
		data->stepInteger = selected->stepInteger;
		return true;
	}
	else
		return false;
}
int         MyClient_GetNumParams()
{
    return nParamInfo;
}
float		MyClient_GetParamValue(int idx, char valueTextOut[AUTOMAP_NAME_LENGTH+1])
{
    if(idx<nParamInfo){
        return paramValues[idx];
    }
    else
        return 0.0;
}

void        MyClient_SetParamValue(int idx, float value)
{
    if(idx<nParamInfo){
		paramValues[idx]=value;
        update[idx]=1;
    }
}

ConnectionHandle *  MyClient_GetConnection()
{
    return m_Connection;
}
EditorHandle *      MyClient_GetEditor()
{
    return &m_Editor;
}


/////////////////////////////////////////////////////////////////////
void SetupEditor()
{
    m_Editor.impl.OnSetCurrentControlInfo = NULL;
    m_Editor.impl.OnLearnChanged =          MyEditor_OnLearnChanged;
    m_Editor.impl.OnAutomapChanged =        MyEditor_OnAutomapChanged;
    m_Editor.impl.OnClearChanged =          MyEditor_OnClearChanged;

    EditorHandle_Initialize(&m_Editor);
}

void SetupClient()
{
    m_This.impl.GetName =           MyClient_GetName;
    m_This.impl.GetMfr =            MyClient_GetMfr;
    m_This.impl.GetType =           MyClient_GetType;
    m_This.impl.GetGUID =           MyClient_GetGUID;
    m_This.impl.GetSaveName =       MyClient_GetSaveName;

    m_This.impl.GetInstanceName =   MyClient_GetInstanceName;
    m_This.impl.SaveInstanceName =  MyClient_SaveInstanceName;

    m_This.impl.GetNumParams =      MyClient_GetNumParams;
    m_This.impl.GetParamInfo =      MyClient_GetParamInfo;
    m_This.impl.GetParamValue =     MyClient_GetParamValue;
    m_This.impl.SetParamValue =     MyClient_SetParamValue;

    m_This.impl.GetNumPresets =     NULL;
    m_This.impl.GetCurrentPreset =  NULL;
    m_This.impl.SetCurrentPreset =  NULL;

    m_This.impl.SupportsTransport = NULL;
    m_This.impl.OnTransport =       NULL;

    m_This.impl.GetConnection =     MyClient_GetConnection;
    m_This.impl.GetEditor =         MyClient_GetEditor;

    ClientHandle_Initialize(&m_This);
}

void Open()
{
    AutomapConnectionStatus stat=ERROR_UNKNOWN;

    if(!opened){
        SetupEditor();
        SetupClient();

        m_Connection = Automap_Connect(&m_This, &stat);

        switch (stat)
        {
        case CONNECTION_OK:         post("Automap: Connection OK");          break;
        case SERVER_TOO_OLD:        post("Automap: Server too old");         break;
        case CLIENT_TOO_OLD:        post("Automap: Client too old");         break;
        case NO_SERVER_INSTALLED:   post("Automap: No server installed");    break;
        case ERROR_UNKNOWN:         post("Automap: Unknown error");          break;
        }

        if (Connection_Initialize(m_Connection))
        {
            post("Automap: Initialized\n");
            opened=1;
        }
        else
        {
            post("Automap: Initialization FAILED!\n");
        }

        Connection_SetAutoFocus(m_Connection);
    }
}

void Close()
{
    if(opened){
        Automap_Disconnect(m_Connection);
        ClientHandle_Destroy(&m_This);
        opened=0;
        nParamInfo=0;
    }
}

///////////////////////// function prototypes
//// standard set
void *automap_max_new(t_symbol *s, long argc, t_atom *argv);
void automap_max_free(t_automap_max *x);
void automap_max_assist(t_automap_max *x, void *b, long m, long a, char *s);
void automap_max_set(t_automap_max *x, t_symbol *s, long argc, t_atom *argv);
void automap_max_register(t_automap_max *x, t_symbol *s, long argc, t_atom *argv);
void automap_max_bang(t_automap_max *x, t_symbol *s, long argc, t_atom *argv);
void automap_max_open(t_automap_max *x, t_symbol *s, long argc, t_atom *argv);
void automap_max_close(t_automap_max *x, t_symbol *s, long argc, t_atom *argv);
void automap_max_reset(t_automap_max *x, t_symbol *s, long argc, t_atom *argv);

//////////////////////// global class pointer variable
void *automap_max_class;


int C74_EXPORT main(void)
{   
    
    // object initialization, NEW STYLE
    t_class *c;
    
    c = class_new("automap_max", (method)automap_max_new, (method)automap_max_free, (long)sizeof(t_automap_max), 
                  0L /* leave NULL!! */, A_GIMME, 0);
    
    /* you CAN'T call this from the patcher */
    class_addmethod(c, (method)automap_max_assist,   "assist", A_CANT, 0);  
    class_addmethod(c, (method)automap_max_set,      "set", A_GIMME, 0);  
    class_addmethod(c, (method)automap_max_register, "register", A_GIMME, 0);  
    class_addmethod(c, (method)automap_max_bang,     "bang", 0);  
    class_addmethod(c, (method)automap_max_open,     "open", 0);  
    class_addmethod(c, (method)automap_max_close,    "close", 0);  
    class_addmethod(c, (method)automap_max_reset,    "reset", 0);  
    
    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    automap_max_class = c;
    return 0;
}

void automap_max_assist(t_automap_max *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { // inlet
        strcpy(s, "Use to send configuration messages. Bang to update the client.");
    } 
    else {  // outlet
        strcpy(s, "Receive the parameter changes with the message 'param'");           
    }
}
int getAsInteger(t_atom *argv){
	switch (atom_gettype(argv))
	{
	case A_LONG:
		return (int)atom_getlong(argv); //yeah doing it the bad way
	case A_FLOAT:
		return (int)atom_getfloat(argv);
	}
	return 0;
}

void automap_max_register(t_automap_max *x, t_symbol *s, long argc, t_atom *argv)
{
    AutomapParamInfo *selected;
	int imax = -1; 
	int imin = -1;
	int istep = -1; 
    
    if(argc!=4){
        object_error(&x->ob,"Register should be sent in the following format: name min max step\n");
    } 
    else{
        if(atom_gettype(argv)!=A_SYM)                                     {object_error(&x->ob,"The first argument need to be a symbol name\n"); return;}
        if(atom_gettype(argv+1)!=A_LONG && atom_gettype(argv+1)!=A_FLOAT) {object_error(&x->ob,"The second argument need to be a number\n");     return;}
        if(atom_gettype(argv+2)!=A_LONG && atom_gettype(argv+2)!=A_FLOAT) {object_error(&x->ob,"The third argument need to be a number\n");      return;}
        if(atom_gettype(argv+3)!=A_LONG && atom_gettype(argv+3)!=A_FLOAT) {object_error(&x->ob,"The fourth argument need to be a number\n");     return;}

		selected = paramInfoArray+nParamInfo;
		strcpy(selected->name, atom_getsym(argv)->s_name); 
		imax = getAsInteger(argv+1); 
		imin = getAsInteger(argv+2);
		istep = getAsInteger(argv+3); 
		selected->maxInteger = imax;
		selected->minInteger = imin;
		selected->stepInteger = istep;
        nParamInfo++;

		//post("Registering %s %i %i %i",selected->name,imax,imin,istep);
    }
}

void automap_max_set(t_automap_max *x, t_symbol *s, long argc, t_atom *argv)
{
    char command[AUTOMAP_NAME_LENGTH];
    char argument[AUTOMAP_NAME_LENGTH];
    if(argc>1){
		if(atom_gettype(argv)!=A_SYM)   {object_error(&x->ob,"The first argument needs to be the a symbol\n"); return;}
        
        strcpy(command, atom_getsym(argv)->s_name);

		if(strcmp(command,"GUID")==0){
		  if(argc==5){
            if(atom_gettype(argv+1)!=A_LONG) {object_error(&x->ob,"Argument 1 is not an integer\n"); return;}
            if(atom_gettype(argv+2)!=A_LONG) {object_error(&x->ob,"Argument 2 is not an integer\n"); return;}
            if(atom_gettype(argv+3)!=A_LONG) {object_error(&x->ob,"Argument 3 is not an integer\n"); return;}
            if(atom_gettype(argv+4)!=A_LONG) {object_error(&x->ob,"Argument 4 is not an integer\n"); return;}
            m_GUID.data1=atom_getlong(argv+1);
            m_GUID.data2=atom_getlong(argv+2);
            m_GUID.data3=atom_getlong(argv+3);
            m_GUID.data4=atom_getlong(argv+4);
          }
          else
             object_error(&x->ob,"GUID is specified as 4 integer numbers\n",command); return;
		}
		else{
    		if(atom_gettype(argv+1)!=A_SYM) {object_error(&x->ob,"The second argument needs to be the a symbol\n"); return;}
            
            strcpy(argument, atom_getsym(argv+1)->s_name);
            
            if(strcmp(command,"name")==0)        { strcpy(clientName, argument); return;} 
            if(strcmp(command,"type")==0)        { strcpy(clientType, argument); return;}  
            if(strcmp(command,"saveName")==0)    { strcpy(saveName, argument); return;} 
            if(strcmp(command,"manufacturer")==0){ strcpy(manufacturer, argument); return;} 
            if(strcmp(command,"instanceName")==0){ strcpy(instanceName, argument); return;} 
            object_error(&x->ob,"'%s' is not a recognized symbol\n",command); return;
		}
    }
    object_error(&x->ob,"Set should be called with two symbols\n",command); return;
}

void automap_max_bang(t_automap_max *x, t_symbol *s, long argc, t_atom *argv)
{
    int i;
    t_atom argvo[2];
    if(opened){                               // check if the connection is open
        Connection_Update(m_Connection);      // Update the parameter values
        for(i=0;i<MAX_PARAM_NUMBER;i++){      // trigger a message for each modified parameter
            if(update[i]!=0){
                atom_setsym(argvo, gensym(paramInfoArray[i].name));
                atom_setfloat(argvo + 1, paramInfoArray[i].minInteger+paramValues[i]*(paramInfoArray[i].maxInteger-paramInfoArray[i].minInteger));
                outlet_anything(x->m_outlet, gensym("param"), 2, argvo);
                update[i]=0;
            }
        }
    }
}
void automap_max_open(t_automap_max *x, t_symbol *s, long argc, t_atom *argv)
{
    Open();
}

void automap_max_close(t_automap_max *x, t_symbol *s, long argc, t_atom *argv)
{
    Close();
}
void automap_max_reset(t_automap_max *x, t_symbol *s, long argc, t_atom *argv)
{
    nParamInfo = 0;
}

void automap_max_free(t_automap_max *x)
{
   Close();
}

void *automap_max_new(t_symbol *s, long argc, t_atom *argv)
{
    t_automap_max *x = NULL;
    
    object_warn(&x->ob,"Do not instantiate more than one automap_max object. This version does not support it.");
    // object instantiation, NEW STYLE
    if (x = (t_automap_max *)object_alloc(automap_max_class)) {
    }
    x->m_outlet = outlet_new((t_object *)x, NULL);

    return (x);
}
