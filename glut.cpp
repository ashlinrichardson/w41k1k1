#include "glut.h"
using namespace myglut;

myglut::GLUTWindow::GLUTWindow(){
	WindowID = false; 
	Update = false;
	myclust = NULL;
}

void myglut::GLUTWindow::focus(){
	glutSetWindow(myID());
}

int myglut::GLUTWindow::initGLUT(int _NRow, int _NCol){
	
	Update = false;
	WindowY = _NRow;  WindowX = _NCol;
	glutInitWindowSize(WindowX, WindowY);
	glutInitDisplayMode(GLUT_RGB);
	WindowID = glutCreateWindow("");
	return WindowID;
}

void myglut::GLUTWindow::setRGB(SA<float> * r, 
                                SA<float> * g, 
                                SA<float> * b, 
								                unsigned int A, 
                                unsigned int B, 
                                unsigned int C){
	b1=r; 
	b2=g; 
	b3=b;
	curband[0]=A;
	curband[1]=B;
	curband[2]=C;
}

void myglut::GLUTWindow::setDisplayFunc( void (*f)()){
	focus();
	glutDisplayFunc(f);
}

void myglut::GLUTWindow::setKeys( void (*f)(unsigned char, int, int) , void (*g)(int, int, int)  ){    // void (*f)(void), void (*f)(void)){
	focus();
	glutKeyboardFunc(f);
	glutSpecialFunc(g);  
}

void myglut::GLUTWindow::setPos(int x, int y){
	focus();
	glutPositionWindow(x,y);		
	XPos = x; YPos = y;
}

void myglut::GLUTWindow::setRightOf( GLUTWindow * other){
	focus();
	setPos( other->XPos + other->WindowX, other->YPos);
}

void myglut::GLUTWindow::setBelow( GLUTWindow * other){
	focus();
	setPos(other->XPos, 40 + other->YPos + other->WindowY);
}

int myglut::GLUTWindow::myID(){
	return WindowID;
}

int myglut::GLUTWindow::nCol(){
	return WindowX;
}

int myglut::GLUTWindow::nRow(){
	return WindowY;
}

int myglut::GLUTWindow::myX(){
	return WindowX;
}

int myglut::GLUTWindow::myY(){
	return WindowY;
}

static void myglut::display3d(){
	(myglut3d)->draw3d();
}

static void myglut::display2d(){
	return;
}

static void myglut::displayd(){
	(myglut2d_img)->refresh();
}

int myglut::GLUT2d::reboot(){
	datClust.init( NRow*NCol*3 );
	datBinary.init( NRow*NCol*3 );
	datResult.init(NRow*NCol);
	indClosest.init(NRow,NCol);
	indCentre.init(NRow,NCol);
	surprise.init(NRow,NCol);	
	int n = NRow*NCol;
	int i;
	for(i=0; i<n; i++){ 
		indClosest[i] =-1;
		indCentre[i]=-1;
		surprise[i]=0;
	}
}

myglut::GLUT2d::GLUT2d(int _NRow, int _NCol){
	thread_exists = false;
	render_clusters = false;
	NRow = _NRow; NCol=_NCol;
	dat = new SA<float>(NRow*NCol*3);
	datClust.init( NRow*NCol*3 );
	datBinary.init( NRow*NCol*3 );
	datResult.init(NRow*NCol);
	indClosest.init(NRow,NCol);
	indCentre.init(NRow,NCol);
	surprise.init(NRow,NCol);
	
	int n = NRow*NCol;
	int i;
	for(i=0; i<n; i++){ 
		indClosest[i] =-1;
		indCentre[i]=-1;
		surprise[i]=0;
	}
	
	GLUTWindow();
	initGLUT(NRow, NCol);
	glutDisplayFunc(displayd);
	glutKeyboardFunc(processKeys);
	glutDisplayFunc(idle);
	
}

int myglut::GLUT2d::initGLUT(int _NRow, int _NCol){
	lock();
	myclust = NULL;
	Update = false;
	isClassification = false;
	WindowY = _NRow;  WindowX = _NCol;
	glutInitWindowSize(WindowX, WindowY);
	glutInitDisplayMode(GLUT_RGB);
	WindowID = glutCreateWindow("");
	return WindowID;
}

void myglut::GLUT2d::setView(){
	focus();
	glViewport(0,0,NCol, NRow);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLfloat)NCol, 0.0, (GLfloat)NRow);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRasterPos2f(0.,0.);
	
}

void myglut::GLUT2d::refresh(){
	if(isClassification && threadcreated) return;
	focus();
	if(Update==true){	
		rebuffer();
		int did_recalc = false;
		if (!isClassification){
			did_recalc=recalc_classes();
		}
		Update = false;
	}
	draw2d();
	glbusy = false;
}

void myglut::GLUT2d::mark(){
	Update=true;
}

void myglut::GLUT2d::draw_classes(){
	beforelaststate = laststate;
	laststate = -2;
	focus();
	setView();	
	glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(NCol, NRow, GL_RGB, GL_FLOAT, (GLvoid *) (datClust.elements) );	
	glFlush();
	glutSwapBuffers();
}

void myglut::GLUT2d::idlethreadfunc(){
	return;
	if(thread_exists) return;
	thread_exists = true;
	int optim=1;
	while(!ostopthread){
		printf("myglut::GLUT2d::idlethreadfunc() iter %d\n", optim++);
		int i;
		int nc = myclust->get_n_knn_centres();
		for(i=0; i<nc; i++){
			PickThis = i; laststate = i;
			source = (GLvoid *)(&((myglut2d->datBinary.elements)[0]));			
			myglut2d->recalc_binary_quick(i);
			myglut2d->recalc_binary(i);
			myglut2d->quickdraw();
			
			
			PickThis = i; laststate = i;
			source = (GLvoid *)(&((myglut2d->datBinary.elements)[0]));			
			myglut2d->recalc_binary_quick(i);
			myglut2d->recalc_binary(i);
			myglut2d->quickdraw();
			
		}
	}
	thread_exists = false;
	
}

void myglut::GLUT2d::draw2d(){
	focus();
	if (!isClassification){
		focus();
		setView();	
		glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
		glDrawPixels(NCol, NRow, GL_RGB, GL_FLOAT, (GLvoid *)(&((dat->elements)[0])));	
		glFlush();
		glutSwapBuffers();		
		return;
	}
	else{
		source = (GLvoid *)(&((dat->elements)[0]));
		if( !render_clusters){
			source = (GLvoid *)(&((dat->elements)[0]));
			
		}
		else{
			if(PickThis < 0){  
				source = (GLvoid *)(&((datClust.elements)[0]));
			}
			else{
				source = (GLvoid *)(&((datBinary.elements)[0]));
				if(PickThis != laststate){
					recalc_binary_quick(PickThis);
				}
				recalc_binary_quick(PickThis);
				quickdraw();
				if( _lastmouseRight ) return;
				
				recalc_classes();
				quickdraw();
				recalc_binary(PickThis);				
				source = (GLvoid *)(&((datBinary.elements)[0]));  //binary classification
			}
		}
		quickdraw();
	}
}

void myglut::GLUT2d::quickdraw(){
	focus();
	setView();	
	glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(NCol, NRow, GL_RGB, GL_FLOAT, source);	
	glFlush();
	glutSwapBuffers();
}

void myglut::GLUT2d::recalc_binary_quick(int mypick){	
	int i,j,k,rs;
	int ind, ind2;
	int result;
	FILE * outfile1 = fopen("output/out_binary.bin", "wb");
	FILE * outfile2 = fopen("output/out_class.bin", "wb");
	
	for(i=0; i<NRow; i++){
		rs = NRow-i-1;

		for(j=0; j<NCol; j++){ 

			ind = ((rs*NCol) + j);    
			ind2 = 3*((rs*NCol) + j);     
			k = datResult.at(ind);
			
			if(k == mypick){
				result = 1;
				datBinary.at(ind2++) = result; 
				datBinary.at(ind2++) = result; 
				datBinary.at(ind2  ) = result; 
			}
			else{
				result =0;
				datBinary.at(ind2++ )  = result; 
				datBinary.at(ind2++ )  = result; 
				datBinary.at(ind2   )  = result; 
			}
			fwrite(&result, sizeof(int), 1, outfile1);
			fwrite(&k,      sizeof(int), 1, outfile2);
 		}
	}
	fclose(outfile1);
	fclose(outfile2);
}	

void myglut::GLUT2d::recalc_binary(int mypick){
	int i,j,k,rs;
	int ind, ind2;
	int comp = NRow/4;
	
	int state = datResult.at(0);
	
  // # of preceding pixels same in spatial ordering
  int count_same_state = 0;  
	
	for(i=0; i<NRow; i++){
		rs = NRow-i-1;
		for(j=0; j<NCol; j++){ 
			
			ind = ((rs*NCol) + j);    
			ind2 = 3*((rs*NCol) + j);     
			k = datResult.at(ind);
			
			surprise.at(i,j) =         count_same_state;
			avg_surprise    += (double)count_same_state;
			
			if(k == state) count_same_state++;
			else{ 
				state=k;
				count_same_state=0;
			}
			
			if( k==mypick){
				datBinary.at(ind2++ ) = 1.; 
				datBinary.at(ind2++ ) = 1.; 
				datBinary.at(ind2   ) = 1.; 
			}
			else{
				datBinary.at(ind2++ ) = 0.; 
				datBinary.at(ind2++ ) = 0.; 
				datBinary.at(ind2   ) = 0.; 
			}
		}
	}	

	float as = (float)(  avg_surprise / ((double)NRow * (double)NCol));
	surprising_count = 0;
	
	for(i=0; i<NRow; i++){
		for(j=0; j<NCol; j++){ 
			if((float)surprise.at(i,j) >= as){
				surprising_count++;
				reclass_point(i,j);
			}
		}
	}
	//printf("Surprising points %d avg. surprise %e\n", surprising_count, as);
}

void myglut::GLUT2d::reclass_point(int i, int j){
	if( !myclust) return;

	vector < SA<float> * > * fb = myclust->float_buffers;
	if (!fb) return;

	int rs = NRow-i-1;
	int cMIN = 0;
	float d, dMIN, tmp;
	int K, J, I, ind, n_elem, rj;
	int N = fb->size();
	int nc = myclust->get_n_knn_centres();
	int rim = myclust->get_Rand_Iter_Max();
	rim = rim * 5;
	
	float x = b1->at(rs,j);
	float y = b2->at(rs,j);
	float z = b3->at(rs,j);
	int rjmin = indClosest.at(i,j);

	if(rjmin >= 0){
		cMIN = indCentre.at(i,j);
		d=0;					
		for(K=0; K<N; K++){
			tmp = (fb->at(K)->at(i,j)) - (myclust->get_centre_coord(cMIN, rjmin, K));
			d += tmp*tmp;
		}
		dMIN = d;
	}
	else{
		dMIN = FLT_MAX;
		cMIN = 0;
	}
	
	for(J=0; J<nc; J++){              
    //for each centre.
		n_elem = myclust->get_n_knn_elements(J);
		
    for(I=0; I<rim; I++){       
    //for each iteration.	
			rj= rand()%n_elem;  //select random element from cluster.
			
			d=0.;					
			for(K=0; K<N; K++){
				tmp = (fb->at(K)->at(i,j)) - (myclust->get_centre_coord(J, rj, K));
				d += tmp*tmp;
			}
			if (d<dMIN){    //found closer element.
				dMIN = d;
				cMIN = J;
				rjmin = rj;
			}
		} 
	} 
	
	// use repr. element from centre to assign color to cluster.
	ind = 3*((rs*NCol) + j);               
	datClust.at(ind++) = myclust->get_centre_coord(cMIN, curband[0]);
	datClust.at(ind++) = myclust->get_centre_coord(cMIN, curband[1]);
	datClust.at(ind  ) = myclust->get_centre_coord(cMIN, curband[2]);
	datResult.at(   (rs*NCol) + j  ) = cMIN;
	
	indClosest.at(i,j) = rjmin;
	indCentre.at(i,j)  = cMIN;
	
}


int myglut::GLUT2d::recalc_classes(){
	printf("myglut::GLUT2d::recalc_classes()\n");
	if( !myclust) return false;
	
  srand ( time(NULL) );
	
	int i,j;
	int I,J,K;
	int p,q,r;
	int N,n,nc;
	int comp, n_elem, rim;
	int cMIN, ind, rs, rj;
	float x, y, z;	
	float d, dMIN,tmp;
	
	vector < SA<float> * > * fb = myclust->float_buffers;
	if (!fb) return false;
	
	nc = myclust->get_n_knn_centres();
	rim = myclust->get_Rand_Iter_Max();
	
	N = fb->size();
	n = NRow*NCol*N;
	
	comp = NRow/20;
	
	int rjmin;
	
	for(i=0; i<NRow; i++){
		rs = NRow-i-1;

		if((i % comp) == 0){
      printf("Applying: %d/100\n", (int)(floor(100.*((float)(i+1)/(float)(NRow)))));
		}

		for(j=0; j<NCol; j++){ 
			x = b1->at(rs,j);
			y = b2->at(rs,j);
			z = b3->at(rs,j);
			rjmin = indClosest.at(i,j);

			if( rjmin >=0){
				cMIN = indCentre.at(i,j);
				d=0;					

				for(K=0; K<N; K++){
					tmp = (fb->at(K)->at(i,j)) - (myclust->get_centre_coord(cMIN, rjmin, K));
					d += tmp*tmp;
				}
				dMIN = d;
			}
			else{
				dMIN = FLT_MAX;
				cMIN = 0;
			}
			
			for(J=0; J<nc; J++){              
        //for each centre.
				n_elem = myclust->get_n_knn_elements(J);
				
				for(I=0; I<rim; I++){       
          //for each iteration
					rj= rand()%n_elem;  //select random element from clust
					
					d=0;					
					for(K=0; K<N; K++){
						tmp = (fb->at(K)->at(i,j)) - (myclust->get_centre_coord(J, rj, K));
						d += tmp*tmp;
					}
					if (d<dMIN){    
            //found closer element.
						dMIN = d;
						cMIN = J;
						rjmin = rj;
					}
				} 
			}
			
			//use repr. element from centre to assign color to clust 
			ind = 3*((rs*NCol) + j);               
			datClust.at(ind++) = myclust->get_centre_coord(cMIN, curband[0]);
			datClust.at(ind++) = myclust->get_centre_coord(cMIN, curband[1]);
			datClust.at(ind  ) = myclust->get_centre_coord(cMIN, curband[2]);
			datResult.at(   (rs*NCol) + j  ) = cMIN;
			
			indClosest.at(i,j) = rjmin;
			indCentre.at(i,j)  = cMIN;
			
		}
	}	
	return true;
}

void myglut::GLUT2d::rebuffer(){
	//focus();
	
	printf("myglut::GLUT2d::rebuffer()\n");
	
	int i,j,k;	
	k=0;
	int ri;
	
	for(i=0; i<NRow; i++){
		ri = NRow-i-1;
		for(j=0; j<NCol; j++){ 
			//remember, computer screen coordinates different..
			dat->at(k++) = b1->at(ri,j);
			dat->at(k++) = b2->at(ri,j);
			dat->at(k++) = b3->at(ri,j);
		}
	}	
}

void myglut::GLUT3d::set_clust( clust_knn * k){
	myclust = k;
}

void myglut::GLUT3d::runclust(){
	myclust->knn_clustering();
}

myglut::GLUT3d::GLUT3d(int _NRow, int _NCol, int _N){
	PickThis = -1;
	myclust = NULL;
	N= _N;
	lock = false;
	writelock = false;
	NRow = _NRow; NCol=_NCol;
  GLUTWindow();
	initGLUT(NRow, NCol);
}

int myglut::GLUT3d::initGLUT(int _NRow, int _NCol){
	Update = false;
	WindowY = _NRow;  WindowX = _NCol;
	glutInitWindowSize(WindowX, WindowY);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glEnable(GL_DEPTH_TEST);
	WindowID = glutCreateWindow("3d Cluster Plot");
	zpr::zprInit();
	zpr::zprSetWindowID(WindowID);
	zpr::zprPickFunc(pick);              /* Pick event client callback   */
	zpr::zprSelectionFunc(display3d);     /* Selection mode draw function */
	glutDisplayFunc(display3d);
	glutKeyboardFunc(processKeys);	
	return WindowID;
}


void myglut::GLUT3d::setView(){
	focus();
}

void myglut::GLUT3d::display_picked_points(int PickThis){
	
	if(!myclust || PickThis < 0) return;
	focus();
		
	/*
	  int get_n_knn_centres(){    //return the number of classes.
	  int get_n_knn_elements(int centre_index){   
        //get the number of dat elements in a given class.
	  float get_centre_coord(int centre_index, int elem_ind, int coord){  
        //get the i'th coordinate of a given N-dimensional dat element.
	 */

	int n_centres = myclust->get_n_knn_centres();
	int i,j, n_elem;
	float x,y,z;
	
	glColor3f(0., 0., 1.);
	glPointSize(3.);
	glBegin(GL_POINTS);
	i = PickThis;

	n_elem =  myclust->get_n_knn_elements(i);
	for(j=0; j<n_elem; j++){
		x = myclust->get_centre_coord(i, j,  (unsigned int)curband[0]);
		y = myclust->get_centre_coord(i, j,  (unsigned int)curband[1]);
		z = myclust->get_centre_coord(i, j,  (unsigned int)curband[2]);
		glVertex3f( x,y,z);
		
	}
	glEnd();	
}

void myglut::GLUT3d::draw3d(){   
	/* update dimensions in common from 3d projection of 4d in 
    multiple windows would be nice */

	focus();
	float x, y, z;
	int i;
	int n = b1->size();
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glColor3f(0., 1., 0.);
	glPointSize(1.);
	
	glBegin(GL_POINTS);

	for(i=0; i<n; i+=20){
		if(i<n){
      glColor3f( b1->at(i), b2->at(i), b3->at(i) ); 
      glVertex3f( b1->at(i), b2->at(i), b3->at(i));
    }
	}
	glEnd();
	
	if( !lock){
		if( class_centres.size()>=N){
			glInitNames();
			
			int j,k;
			k=0;
			SA<float> xyz(N);
			int nn = class_centres.size()/N;
			if(nn != number_of_classes){
				printf("Error: count of number of classes did not match.\n");
				exit(1);
			}

			for(i=0; i<nn; i++){
				glPushName(i);

				for(j=0; j<N; j++){
					xyz[j]=class_centres[k++];
				}

				x = xyz[(unsigned int)curband[0]];
				y = xyz[(unsigned int)curband[1]];
				z = xyz[(unsigned int)curband[2]];
				if(PickThis==i) 
          glColor3f(1., 1., 1.);
				else  
          glColor3f(0., 0., 1.);
				glPushMatrix();
				glTranslatef((GLfloat)x,(GLfloat)y,(GLfloat)z);
				glutSolidSphere(0.02, 10, 10);
				glPopMatrix();
				glPopName();
			}
		}
	}
	
	if(PickThis >= 0){
		display_picked_points(PickThis);
	}
	
	console::drawText();
	glFlush();
	glutSwapBuffers();
	PickThis=-1;
}

void myglut::GLUT3d::refresh(){
	draw3d();
}
void myglut::GLUT3d::mark(){
	Update=true;
}

static void myglut::processKeys(unsigned char key, int x, int y) {
	console::keyboard(key, x, y);
}

void extern myglut::show_classes(){   
	myglut2d->draw_classes();
}

void extern myglut::toggle_display(){   
	if( (beforelaststate != laststate) && (beforelaststate<number_of_classes)&&(beforelaststate>=0) ){
		int tmp;
		tmp = laststate;
		laststate = beforelaststate;
		beforelaststate = tmp;
		
		source = (GLvoid *)(&((myglut2d->datBinary.elements)[0]));
		if(PickThis != laststate){
			myglut2d->recalc_binary_quick(laststate);
		}
		myglut2d->quickdraw();
	}
	else if((beforelaststate != laststate) && ((beforelaststate==-2) || (laststate==-2))){
		int tmp;
		tmp = laststate;
		laststate = beforelaststate;
		beforelaststate = tmp;
		
		if( beforelaststate==-2){
			source = (GLvoid *)(&((myglut2d->datBinary.elements)[0]));
			if(PickThis != laststate){
				myglut2d->recalc_binary_quick(laststate);
			}
			myglut2d->quickdraw();
		}
		else if(laststate==-2){
			source = (GLvoid *)(&((myglut2d->datClust.elements)[0]));
			if(PickThis != laststate){
				myglut2d->recalc_binary_quick(laststate);
			}
			myglut2d->quickdraw();
		}
	}
}
