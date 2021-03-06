/*********************** self documentation **********************/
/*
 * graph_draw.c - Subroutines to draw a graph representing a 
		  sequence of SU applications with TCL/TK
 *
 *
 */
/**************** end self doc ********************************/

/*
 * AUTHOR:  Alejandro E. Murillo, Colorado School of Mines, 03/03/96
 *
*/

/*

Copyright statement:
Copyright (c) Colorado School of Mines, 1995,1996
All rights reserved.

*/


#include "dsu.h"

double    link_angles[] = { 0., PI/4., -PI/4., PI/2., -PI/2.};
double    link_scale[] = { 125., 125., 150., 125., 150.};


void move_children_links(TASK T)
{
  int i;

  for ( i = 0; i < MAX_LINK; i++)

    if ( (T -> next[i]) != (TASK)NULL ) { /* Move children links */

      X2_COORD( T->next[i]->links[MAIN_LINK]->link ) = X_COORD(T->icon);
      Y2_COORD( T->next[i]->links[MAIN_LINK]->link ) = Y_COORD(T->icon);
      SET_COORDS(interp, MAIN_GRAPH -> GRAPH_C,
          T->next[i]->links[MAIN_LINK]->link);

    }

} /* End of move_children_links */

void update_position(int link)
{
  double xincr, yincr;
  TASK active;

  if (MAIN_GRAPH->currpos == NULL) {

    MAIN_GRAPH -> currpos = create_position();

    return;

  }


  if ( link == 0 ) {

    xincr = 125;
    yincr = 0;

  } else {

    /* APPLY an "angle[link] rotation" to the point (x0 + 150, y0)  */

    xincr = link_scale[link]*cos(link_angles[link]);
    yincr = -link_scale[link]*sin(link_angles[link]);

  }

  MAIN_GRAPH->currpos->curX = 
	X_COORD(MAIN_GRAPH ->actv_task -> icon) + (int) xincr;
  MAIN_GRAPH->currpos->curY = 
	Y_COORD(MAIN_GRAPH ->actv_task -> icon) + (int) yincr;

} /* End of update_position() */
      

TASK get_task_icon( TASK T, int index )
{
  TASK T1;
  int  i;

  if ( T == (TASK)NULL) return( (TASK) NULL );

  if ( T->icon->id == index ) return( T );
  if ( T->nme->id == index ) return( T ); /* ADDED 03/02/95 */

  for (i = 0; i < MAX_LINK; i++)
    if ( (T1 = get_task_icon(T -> next[i], index)) != (TASK) NULL) return(T1);

  return( (TASK) NULL );

} /* End of get_task_icon */
   

TASK get_task_index( index )
int index;
{
  TASK T;

  T = MAIN_GRAPH->task_list;
  return( get_task_icon(T, index) );
}


int create_graph_task( T, link )
TASK T;
int link;
{
	GOBJ G;

	char *graph_empty_color;
	char *graph_fg_color;

	char cmdopts[1024];
	char cmd[1024];

	int graph_size;
	int x, y;

	REFRESH_GLOBAL( TASK_NOSETUP_COLOR );
	REFRESH_GLOBAL( GRAPH_FG_COLOR );

	/* 
		Determine the position in the canvas for this task
	*/

	update_position(link);

	x = MAIN_GRAPH -> currpos -> curX;
	y = MAIN_GRAPH -> currpos -> curY;

/* Undraw Any Old Task

	if ( T->icon != NULL )
		destroy_graph_task( T);
*/

/* Create Icon */


	CREATE_RECT( interp, MAIN_GRAPH -> GRAPH_C, T->icon, 
			x-50, y-15, x+50, y+15,
        		CHAR_GLOBVAL( TASK_NOSETUP_COLOR ),
        		CHAR_GLOBVAL( GRAPH_FG_COLOR ) );

	X_COORD( T->icon ) = x;
	Y_COORD( T->icon ) = y;
	T->icon->color = T -> color;

	/* Create Name */

	T->nme = create_gobj();
	sprintf( cmd,
		"%s create text %d %d -text %s -anchor center -fill %s -tags node",
		/* MAIN_GRAPH -> GRAPH_C, x, y + 60, T->name, */
		MAIN_GRAPH -> GRAPH_C, x, y, T->name,
		CHAR_GLOBVAL( GRAPH_FG_COLOR ) );

	Tcl_Eval( interp, cmd );
	T->nme->id = atoi( interp->result );
	X_COORD( T->nme ) = x;
	Y_COORD( T->nme ) = y;
	T->nme->color = CHAR_GLOBVAL( GRAPH_FG_COLOR );

	/* Create Link */

	if (T -> parent != (TASK) NULL) {
	  T->links[MAIN_LINK] = create_graphlink();
	  CREATE_GRAPH_LINK( interp, MAIN_GRAPH->GRAPH_C,
		T->links[MAIN_LINK]->link,
		x, y, X_COORD(T->parent->icon), Y_COORD(T->parent->icon), 
		CHAR_GLOBVAL( GRAPH_FG_COLOR ) );
	}

	move_children_links(T);

}

destroy_graph_task( T )
TASK T;
{
  char cmd[1024];

  if ( T->icon != NULL )
  {

    sprintf( cmd, "%s delete %d %d",
    GRAPH_C,  T->icon->id, T->nme->id );
    Tcl_Eval( interp, cmd );


    if ( T-> parWin !=  NULL)
    {
      sprintf( cmd, "wm withdraw %s", T->parWin);
      Tcl_Eval( interp, cmd );
    }

    if ( T->links[MAIN_LINK] != (GRAPHLINK) NULL )
    {
      destroy_link(T);
    }

    free_gobj( &(T->icon) );
    free_gobj( &(T->nme) );

    if ( T->links[MAIN_LINK] != (GRAPHLINK) NULL )
      free_gobj( &(T->links[MAIN_LINK]->link) );
    T ->links[MAIN_LINK] = (GRAPHLINK) NULL; /* AEM 08/21/95 */
  }

}


set_task_color( T, color )
TASK T;
char *color;
{
	char cmd[1024];

	SET_TASK_COLOR(interp, GRAPH_C, T, color, 2);

	if ( !strcmp( color, "blue" ) )
	{
		sprintf( cmd, "%s itemconfigure %d -foreground white",
			GRAPH_C, T->icon->id );
		Tcl_Eval( interp, cmd );
	}

	else
	{
		sprintf( cmd, "%s itemconfigure %d -foreground blue",
			GRAPH_C, T->icon->id );
		Tcl_Eval( interp, cmd );
	}
}

int align_with_T(TASK T, double X, double Y)
{
  int  i, newX, newY;

  if (T == (TASK)NULL ) return;

  for (i = 0; i < MAX_LINK; i++)
    
    if (T -> next[i] != (TASK)NULL) {
      newX = X + link_scale[i]*cos(link_angles[i]);
      newY = Y - link_scale[i]*sin(link_angles[i]);
      redrawNodeC( T -> next[i], newX - X_COORD(T -> next[i] -> icon),
				 newY - Y_COORD(T -> next[i] -> icon) );
      align_with_T(T -> next[i], newX, newY);
    }

  return(TCL_OK);
}


int moveNodeC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  int canvx, canvy;
  int id;

  id = atoi( argv[1] );
  canvx = atoi( argv[2] );
  canvy = atoi( argv[3] );

  T = get_task_index(id);
  if (T != (TASK)NULL ) {
    redrawNodeC( T, canvx, canvy);
    align_with_T(T, X_COORD(T->icon), Y_COORD(T->icon) ); /* AEM 12/03 */
  }
  return(TCL_OK);
}

int redrawNodeC( T, canvx, canvy)
TASK T;
int canvx, canvy;
{

  char cmd[1024];

  sprintf( cmd, "%s move %d %d %d", GRAPH_C, T->icon->id, canvx, canvy);
  Tcl_Eval( interp, cmd );
  sprintf( cmd, "%s move %d %d %d", GRAPH_C, T->nme->id, canvx, canvy);
  Tcl_Eval( interp, cmd );
  X_COORD( T->icon ) += canvx;
  Y_COORD( T->icon ) += canvy;
  X_COORD( T->nme ) += canvx;
  Y_COORD( T->nme ) += canvy;

  if ( T == MAIN_GRAPH -> actv_task)
    set_task_color(T, CHAR_GLOBVAL(TASK_ACTIVE_COLOR) );
  else
    set_task_color( T, T -> color);

  if (T -> parent != (TASK)NULL) { /* Move parent link */

    X1_COORD( T->links[MAIN_LINK]->link ) = X_COORD( T -> icon);
    Y1_COORD( T->links[MAIN_LINK]->link ) = Y_COORD( T -> icon);

/* AEM: 08/20/95: needed when deleting */

    X2_COORD( T->links[MAIN_LINK]->link ) = X_COORD( T -> parent-> icon);
    Y2_COORD( T->links[MAIN_LINK]->link ) = Y_COORD( T -> parent-> icon);

    SET_COORDS(interp, GRAPH_C, T->links[MAIN_LINK]->link);
  }

  move_children_links(T); /* Move child link */

}

int set_active_task( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  char cmd[1024];

  REFRESH_GLOBAL( TASK_SETUP_COLOR ); 
  REFRESH_GLOBAL( TASK_NOSETUP_COLOR );
  REFRESH_GLOBAL( TASK_ACTIVE_COLOR );

  T = get_task_index(atoi(argv[1]));
  
  if (T != (TASK)NULL) 
    set_the_active(T);

  return(TCL_OK);
}

int set_the_active(TASK T)
{

  if (T != (TASK)NULL) {

    if ( MAIN_GRAPH->actv_task != (TASK)NULL)
      set_task_color( MAIN_GRAPH->actv_task, 
		       MAIN_GRAPH->actv_task -> color);

    set_task_color( T, CHAR_GLOBVAL( TASK_ACTIVE_COLOR ));

    MAIN_GRAPH->currpos->curX = X_COORD(T->icon);
    MAIN_GRAPH->currpos->curY = Y_COORD(T->icon);

  }

  MAIN_GRAPH->actv_task = T; /* update active */

  return(TCL_OK);
}

int show_task_help( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  char cmd[1024];

  T = get_task_index(atoi(argv[1]));

  if (T != (TASK)NULL) {
    sprintf(cmd, "showHelp .help%s %s %s\n", T -> name, 
					    T -> helpFile, 
					    T -> name);
    Tcl_Eval( itp, cmd );
    return(TCL_OK);
  }
  return(TCL_ERROR);

}



/*
	DRAW the new created task 
*/

int draw_graph_task(TASK T, int link)
{

  REFRESH_GLOBAL( TASK_NOSETUP_COLOR );
  REFRESH_GLOBAL( TASK_SETUP_COLOR );
  REFRESH_GLOBAL( TASK_ACTIVE_COLOR );

/*
  printf("\tDrawing Task (%d) %s ... ", link, T -> name);
  fflush(stdout);
*/

  if (T ->status == TASK_SETUP)
      T -> color = CHAR_GLOBVAL(TASK_SETUP_COLOR);
  else
      T -> color = CHAR_GLOBVAL(TASK_NOSETUP_COLOR);

  create_graph_task(T, link); /* Create the GOBJ's for this task */

  set_the_active(T); /* update active */

/* Redraw the graph from this T */

  align_with_T(T, MAIN_GRAPH->currpos->curX, MAIN_GRAPH->currpos->curY);

/*
  printf(" Ready \n");
  fflush(stdout);
*/

  return(TCL_OK);
}

int createParsWinC (TASK T)
{
  char cmd[2048];
  int i; 

  /* Create/Save the window name */

  sprintf(cmd, ".w%s%d\0", T -> name, T->icon->id);
  T -> parWin = copy_str(cmd);

  sprintf(cmd, "createArgvWin %s %s %d \0", 
	T -> parWin, 
	T -> name, 
	T->icon->id);

  for (i = 0; i < T -> S -> argc; i++) {

      strcat(cmd, "\"");

      strcat(cmd, T -> argvlabels[i]);
      strcat(cmd, "=");

      if (T -> argv[i] != (char *)NULL)
        strcat(cmd, T -> argv[i]);
      else
        strcat(cmd, "");

      strcat(cmd, "\" ");

  }

/*
  fprintf(stderr, "Executing  (%s)\n\n", cmd);
  fflush(stderr);
*/

  Tcl_Eval(interp, cmd);

}

/*
  mkNodeC: 
	argv[1]: application name
	argv[2]: fork ( 0 means no fork)
*/

int mkNodeC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  int fork, link;

  /* Get this application (argv[1]) information */

  if ( (T = create_task_nme(argv[1])) == (TASK)NULL) {
    fprintf(stderr, "\tThere is no INFO for this application: %s\n", 
	argv[1]);
    return(TCL_ERROR);
  }

  /* Add this task to the graph structure */

  fork = atoi(argv[2]);
  link = add_task_graph(MAIN_GRAPH, T, fork);

  /* Create the GOBJS for this task */

  draw_graph_task(T, link);
  createParsWinC(T); /* Build the argument windows */

  return(TCL_OK);
}


int getParsC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  char cmd[1024];

  REFRESH_GLOBAL( TASK_SETUP_COLOR );

  T = get_task_index(atoi(argv[1]));

  if (T != (TASK)NULL ) {

    sprintf(cmd, "showWin %s", T -> parWin);
    Tcl_Eval(itp, cmd);

/*
    sprintf(cmd, "raise %s", T -> parWin);
    Tcl_Eval(itp, cmd);
*/

    T -> status = TASK_SETUP;
    T -> color  = CHAR_GLOBVAL(TASK_SETUP_COLOR);

  }
  return(TCL_OK);
}

int saveParsC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  int i, tmpargc;
  char **tmpargv;

  T = get_task_index( atoi(argv[1]) );

  if (T != (TASK)NULL ) {

    tmpargv = (char **)NULL;
    T -> argc = 0;

    if (Tcl_SplitList(interp, argv[2], &tmpargc, &tmpargv) != TCL_OK)
      return TCL_ERROR;

    for (i = 0; i < tmpargc; i++) {

      if ( T -> argv[i] != (char *)NULL) {
	free(T -> argv[i]); 
        T -> argv[i] = (char *)NULL;
      }

      if ( !compare("-@-", tmpargv[i]) ) 
      {
        /* It has been modified */

        T -> argv[i] = copy_str(tmpargv[i]);
        T -> argc++;
      }

    }

    if (tmpargv != (char **)NULL) free((char **)tmpargv);
  }
  return(TCL_OK);

}

GRAPHLINK create_graphlink()
{
        GRAPHLINK tmp;

        tmp = (GRAPHLINK) malloc( sizeof( struct graphlink_struct ) );
        memcheck( tmp, "Graph Link Structure" );
        tmp->link = (GOBJ) NULL;

        return( tmp );
}

free_graphlink( ptr )
GRAPHLINK *ptr;
{
        GRAPHLINK L;

        L = *ptr;

        L->link = (GOBJ) NULL;

        free( *ptr );
        *ptr = (GRAPHLINK) NULL;
}


POSITION create_position()
{
  POSITION tmp;

  REFRESH_GLOBAL( GRAPH_SIZE );
  
  tmp = (POSITION) malloc( sizeof( struct position ) );
  memcheck( tmp, "Position Structure" );
  
  tmp->curX = -1*(INT_GLOBVAL( GRAPH_SIZE ) / 2) + 100;
  tmp->curY = 0;
  tmp->incX = 150;
  tmp->incY = 0;
	
  return( tmp );
}

free_position( ptr )
POSITION *ptr;
{
        POSITION P;

        P = *ptr;

        free( *ptr );
        *ptr = (POSITION) NULL;
}


destroy_link( T )
TASK T;
{
  char cmd[1024];

  if ( T != (TASK)NULL )
    if ( T->links[MAIN_LINK] != (GRAPHLINK) NULL )
    {
      sprintf(cmd,"%s delete %d",GRAPH_C, T->links[MAIN_LINK]->link->id );
      Tcl_Eval( interp, cmd );
      free_gobj( &(T->links[MAIN_LINK]->link) );
      T->links[MAIN_LINK] = (GRAPHLINK) NULL;
    }
}

TASK next_avail(TASK T)
{
  int i;

  for (i = 0; i < MAX_LINK; i++)
    if (T -> next[i] != (TASK)NULL) return (T -> next[i]);

  return((TASK)NULL);

}

void update_parent_next(TASK T, TASK TNEXT)
{
  int i;

  for (i = 0; i < MAX_LINK; i++)

    if (T -> parent -> next[i] == T ) {

      T -> parent -> next[i] = TNEXT;

      if (TNEXT == (TASK)NULL) 
	T -> parent -> nbranches--;
	
      return;
    }

}


int delNodeC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T, TNEXT;

  T = MAIN_GRAPH -> actv_task;

  if (T != (TASK)NULL ) {

    if (T -> nbranches > 1 ) {

      printf("Delete the branches before deleting this node\n");
      Tcl_Eval( interp, "setMsg \"This node cannot be deleted !! \"" );
      return(TCL_OK);
  
    }

/*
     dump_branch_info(stderr, T); AEM
*/

    /* Update pointers */

    TNEXT = next_avail(T);  /* The only one possible followin T */

    if ( MAIN_GRAPH -> task_list == T ) /* the first in the list */
      MAIN_GRAPH -> task_list = TNEXT;

    if (T -> parent != (TASK)NULL) {

      set_the_active(T -> parent);
      update_parent_next(T, TNEXT);
      
    } else 

      set_the_active(TNEXT);

    if (TNEXT != (TASK)NULL) TNEXT -> parent = T -> parent;


    /* Destroy this node graphic information graphic information */

    destroy_graph_task(T);

    if (MAIN_GRAPH -> task_list == MAIN_GRAPH -> actv_task)
      destroy_link(MAIN_GRAPH -> actv_task);

    /* Redraw the graph */

    align_with_T(MAIN_GRAPH -> actv_task, 
		 MAIN_GRAPH->currpos->curX, 
		 MAIN_GRAPH->currpos->curY);

    if ( MAIN_GRAPH -> actv_task == (TASK)NULL)
      free_position(&MAIN_GRAPH -> currpos); /*The graph is empty !!! */
					     /* This can be eliminated
						if we ask for link == -1 in
						add_graph_node() */

  }
  return(TCL_OK);

}

int printGraphInfo( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;

  T = MAIN_GRAPH -> actv_task;

  if (T != (TASK)NULL)
    dump_branch_info(stderr, T);

  /* Print host information (OPTIONAL) */
  initialize_hosts();
  dump_host_info(MAIN_INFO -> host_list);

  return(TCL_OK);

}

/*

  setafieldC:
	argv[1]: node index
	argv[2]: field to be setup
		0 -- stdin
		1 -- stdout
		2 -- hostname
		3 -- options
	argv[3]: value of the field

*/

int setafieldC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  int  i, option;

  T = get_task_index(atoi( argv[1] ));

  option = atoi(argv[2]);

  if (T != (TASK)NULL ) {

    switch ( option) {

      case 0: /* Check if it is legal to setup STDIN in this node */

        if (T -> parent != (TASK)NULL) return(TCL_ERROR);
        if (T -> srcfn != (char *)NULL) free(T -> srcfn); /* Clean */

	T -> srcfn = (char *)NULL;
        T -> srcfd = 0;

        if (!compare(argv[3],"\0") ){
	  T -> srcfn = copy_str(argv[3]);
          T -> srcfd = -1;
	}

        break;

      case 1: /* Check if it is legal to setup STDOUT in this node */

        for (i = 0; i < MAX_LINK; i++)
          if (T -> next[i] != (TASK)NULL) return(TCL_ERROR);

        if (T -> dstfn != (char *)NULL) free(T -> dstfn);

        T -> dstfd = 1;
	T -> dstfn = (char *)NULL;

        if (!compare(argv[3],"\0") ) {
	  T -> dstfn = copy_str(argv[3]);
          T -> dstfd = -1;
	}
        break;

      case 2: /* Set hostname */

        if (T -> myhost != (char *)NULL) free(T -> myhost);
        if (!compare(argv[3],"\0") ) T -> myhost = copy_str(argv[3]);
	else T -> myhost = (char *)NULL;
        break;

      case 3: /* Set options */

        if (T -> options != (char *)NULL) free(T -> options);
        if (!compare(argv[3],"\0") ) T -> options = copy_str(argv[3]);
	else T -> options = (char *)NULL;

    } /* End of switch */

  }
  
  return(TCL_OK);
}

/*

  getafieldC:
	argv[1]: node index
	argv[2]: field requested
		0 -- stdin
		1 -- stdout
		2 -- hostname
		4 -- options
		5 -- npars (T -> S -> argc)
		6 -- sourceCode (T -> S -> sourceCode)
	The output will be left in the TCL global variable: myresult

*/

int getafieldC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  TASK T;
  int  field;
  char varname[256], varvalue[1024];

  T = get_task_index(atoi( argv[1] ));

  field = atoi(argv[2]);

  if (T != (TASK)NULL ) {

    strcpy(varvalue, "\0");

    switch (field) {

      case 0: /* STDIN requested */

        if (T -> srcfn != (char *)NULL)
          strcpy(varvalue, T -> srcfn);
        break;

      case 1: /* STDOUT requested */

        if (T -> dstfn != (char *)NULL)
          strcpy(varvalue, T -> dstfn);
        break;

      case 2: /* Get hostname */

        if (T -> myhost != (char *)NULL)
          strcpy(varvalue, T -> myhost);
        break;

      case 3: /* Get options */

        if (T -> options != (char *)NULL)
          strcpy(varvalue, T -> options);
        break;

      case 4: /* Get npars */

        sprintf(varvalue, "%d\0", T -> S -> argc);
        break;

      case 5: /* Get application name */

        if (T -> name != (char *)NULL)
          strcpy(varvalue, T -> name);
        break;

      case 6: /* Get source code directory name */

        if (T -> S -> sourceCode != (char *)NULL)
          strcpy(varvalue, T -> S -> sourceCode);
        break;

    } /* End of switch */

    strcpy(varname, "myresult");
    Tcl_SetVar(itp, varname, varvalue, TCL_LEAVE_ERR_MSG);
  }
  
  return(TCL_OK);
}

int getsrcdirC( clientData, itp, argc, argv )
ClientData clientData;
Tcl_Interp *itp;
int argc;
char **argv;
{
  DSUAPPL S;
  int  field;
  char varname[256], varvalue[256];

  S = getThisAplInfo(argv[1]);

  if (S != (DSUAPPL)NULL)
    strcpy(varvalue, S -> sourceCode);
  else
    sprintf(varvalue, "cwp/lib\0"); /* Can return su/lib as well */

  strcpy(varname, "myresult");
  Tcl_SetVar(itp, varname, varvalue, TCL_LEAVE_ERR_MSG);

  return(TCL_OK);
}
