/*Name: Vincent Wu
  csid: vncentwu
 */

#include <stdlib.h>
//#include <glut.h>
#include <GL/glut.h> //for linux
//#include <glut.h> //windows
//#include <math.h>
#include <stdio.h>
//#include <string>
#include <pthread.h>
#include <iostream>
//#include <thread>
//#include <future>
#include "loader.h"
#include "main.h"
#include <fstream>
//#include <pthread.h>
#include <unistd.h>
//#include "src/include/GL/glui.h"
//#include "glui-2.36/src/lib/libglui.a"
//#include <cstring>
//#include "GL/glui.h"
//#include "libglui.a"
 // #include <GL/glui.h>
#include "src/include/GL/glui.h"


using namespace std;
#define LINE_SIZE    1024
#define VERSIONA	 2
#define VERSIONB	 3 
#define PI 3.14159265

vector<float> camera_target;
bool ready;
bool lighting_off;
int mode;
bool local_coords;
bool fnormals;
bool vnormals;
GLUI *glui, *glui2;
GLUI_Spinner    *light0_spinner, *light1_spinner;
GLUI_RadioGroup *radio;
GLUI_Panel      *obj_panel, *create_panel, *edit_panel, *transform_panel, 
*attribute_panel, *geometry_panel, *detail_panel, *motion_panel;
GLUI_EditText	*edit_node_name, *model_name, *x_text, *y_text, *z_text, *rotation_text, 
	*cur_name_text, *cur_type_text, *cur_id_text, *cur_depth_text, *cur_parent_text;
GLUI_TextBox *tree_display;
GLUI_List *gui_node_list;
GLUI_Listbox *type_selector, *attribute_list, *transformation_list;
GLUI_Spinner *x_spinner, *y_spinner, *z_spinner, *rotation_spinner;
float x_val, y_val, z_val, rotation_val;



vector<Node*> tree_list;
Node *current_node, *main_camera;
string cur_name_textx = "";
string cur_type_textx = "ROOT";  
string cur_parent_textx = "";
string model_namex = "";
string add_text = "";
string dummy_text = "";
int cur_id_textx, cur_depth_textx, curr_trans_index;
bool update_lock;
int hide_node;

int   wireframe = 0;
int   obj_type = 1;
int   segments = 8;
int   segments2 = 8;
int   light0_enabled = 1;
int   light1_enabled = 1;
float light0_intensity = 1.0;
float light1_intensity = .4;
int   main_window;
float scale = 1.0;
int   show_sphere=1;
int   show_torus=1;
int   show_axes = 1;
int   show_text = 1;
float sphere_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float torus_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float obj_pos[] = { 0.0, 0.0, 0.0 };
char *string_list[] = { "Hello World!", "Foo", "Testing...", "Bounding box: on" };
int   curr_string = 0;
int   curr_attr = 6;
int   curr_type_string = 0;
int motion_type = 0;

int counter;
Node* root;

vector<GLenum> all_lights {GL_LIGHT7, GL_LIGHT6, GL_LIGHT5, GL_LIGHT4, GL_LIGHT3, 
	GL_LIGHT2, GL_LIGHT1, GL_LIGHT0};
vector<GLenum> free_lights {GL_LIGHT7, GL_LIGHT6, GL_LIGHT5, GL_LIGHT4, GL_LIGHT3, 
	GL_LIGHT2, GL_LIGHT1, GL_LIGHT0};


void* wait_in(void*)
{
}


void update_tree_list()
{
	gui_node_list->delete_all();
	tree_list.clear();
	root->getTreeText(&tree_list, 0);
	int pd = 0;
	counter = 0;
	int counters[20];
	int ind = 0;
	for(int i = 0; i < tree_list.size(); i++)
	{
		string temp = "";
		Node* elem = tree_list[i];		
		if(elem->n_depth > pd)
		{	
			if(ind >= 20)
			{
				cout << "Error: tree max depth of 20" << endl;
				return;
			}
				
			counters[ind] = counter;
			ind++;
			counter = 0;
		}
		else if(elem->n_depth < pd)
		{
			ind--;
			counter = counters[ind];
		}
			
		counter++;		
		for(int j = 0; j < elem->n_depth; j++)
			temp = temp + "   ";
		temp = temp + to_string(counter) + ". ";
		temp = temp + elem->n_name;
		if(elem->isDefaultName)
			temp = temp + " (ID: " + to_string(elem->n_id) + ")"; 
		gui_node_list->add_item(i, temp.c_str());
		pd = elem->n_depth;
	}
}

void hide_cb(int mode)
{
	current_node->hide = hide_node;
}

void dummy_func(int mode)
{
	//cout << "HIIIIIIIIIIIIIIIIIIIII" << endl;
}

void motion_cb(int mode)
{
	current_node->motion_type = motion_type;
	current_node->tick = 0;
	//cout << "setting motion type to " << motion_type << endl;
}

void name_cb(int mode)
{
	return;
	//cout << "callback for: " << current_node->n_name << " to " << cur_name_textx.c_str() << endl;
	if(mode == 0)
	{
		string temp = cur_name_textx;
		current_node->n_name = temp.c_str();
		//cout << "curr name is now: " << current_node->n_name << endl;
	}
		
}

void toggle_transform(int mode)
{
	if(mode == 0)
		transform_panel->disable();
	else
		transform_panel->enable();
}

void toggle_model_text(int mode)
{
	if(mode == 0)
		geometry_panel->disable();
	else
		geometry_panel->enable();
}

void toggle_attributes(int mode)
{
	if(mode == 0)
		attribute_panel->disable();
	else
		attribute_panel->enable();
}

void toggle_motion(int mode)
{
	if(mode == 0)
		motion_panel->disable();
	else
		motion_panel->enable();
}

void toggle_trinity(int mode)
{
	toggle_transform(mode);
	toggle_attributes(mode);
	toggle_model_text(mode);
	toggle_motion(mode);
}

void update_current()
{
	hide_node = current_node->hide;
	cur_name_textx = current_node->n_name;
	cur_type_textx = (current_node->n_depth > 0) ? 
		node_type_string[current_node->type]	 :
		"ROOT";

	cur_id_textx = current_node->n_id;
	cur_depth_textx = current_node->n_depth;
	x_val = current_node->x;
	y_val = current_node->y;
	z_val = current_node->z;
	transformation_list->set_int_val(current_node->transformation_type);
	rotation_val = current_node->degree;
	model_namex = current_node->model_name;

	if(current_node->attr_type)
	{
		attribute_list->set_int_val(current_node->attr_type);
	}
	if(current_node->parent)
	{
		string temp = "";
		if(current_node->isDefaultName)
			temp = temp + "(";
		cur_parent_textx = current_node->parent->n_name;
	}
	else
		cur_parent_textx = "";
	toggle_trinity(0);
	if(current_node->type == TRANSFORMATION)
	{
		toggle_transform(1);
		transform_panel->set_name("Transformations");
	}
	else if(current_node->type == OBJECT)
	{
		toggle_transform(1);
		transformation_list->disable();
		current_node->transformation_type = TRANSLATE;
		transform_panel->set_name("Coordinate space");
	}
	else if(current_node-> type == CAMERA)
	{
		toggle_transform(1);
		transform_panel->set_name("Projection (square)");
	}
	else if(current_node->type == ATTRIBUTE)
		toggle_attributes(1);
	else if(current_node->type == GEOMETRY)
		toggle_model_text(1);
	else if(current_node->type == MOTION)
	{
		toggle_motion(1);
		toggle_transform(1);
		transform_panel->set_name("Motion parameters");
	}
}

void control_cb(int control)
{

	//reset tree
	if(control == 1)
	{
		root->children.clear();
		current_node = root;
		update_current();
		update_tree_list();

	}
	return;
}

void add_node(int mode)
{		

	if(mode != 0 && current_node->n_depth == 0)
	{
		cout << "Cannot delete root" << endl;
		return;
	}
	const char* nam = edit_node_name->get_text();
	int line = gui_node_list->get_current_item();
	int type = curr_type_string;
	Node *parent = current_node->parent;
	if(strlen(nam) == 0)
		nam = node_type_string[type];
	Node *node = new Node(type, nam);
	bool no_children;
	if(node->type == LIGHT)
	{

		node->light = free_lights.back();
		free_lights.pop_back();
		//cout << "GIVING LIGHT: " << node->light << endl;

	}
		

	if(mode == 0) //as child
	{
		current_node->addChild(node);
	}
	else if(mode == 1) //as parent
	{
		parent->addChild(node);
		parent->removeChild(current_node);
		node->addChild(current_node);
		current_node->increment_depth();
		current_node = node;
	}
	else if(mode == 2) //delete node
	{
		if(current_node->type == LIGHT)
		{
			glDisable(current_node->light);
			free_lights.push_back(current_node->light);
			//cout << "disabling light" << endl;
		}
			
		parent->removeChild(current_node);	
		if(current_node->children.size() == 1)
		{
			current_node = current_node->children[0];
			parent->addChild(current_node);
			//current_node->decrement_depth_children();
		}
		else if(current_node->children.size() == 0)
		{
			no_children = true;
			current_node = parent;
		}	
		else
		{
			cout << "Cannot delete node with more than one child." << endl;	
			if(current_node->type == LIGHT)
				free_lights.push_back(node->light);
		}
			

	}
	update_tree_list();
	gui_node_list->update_and_draw_text();
	if(mode == 0)
		gui_node_list->curr_line = line + 1;
	else if(no_children)
		gui_node_list->curr_line = line;
	else if(mode == 1 | mode == 2)
		gui_node_list->curr_line = line;
	if(mode != 2)
		current_node = node;
	//cout << gui_node_list->curr_line << endl;
	update_current();
	add_text = "";
}

void load_model(int mode)
{
	//cout << "this?" << endl;
}



void process_transform(int mode)
{

}

void attribute_cb(int mode)
{
	if(mode == 0)
	{
		//cout << "gave node attr type: " << curr_attr << endl;
		current_node->attr_type = curr_attr;
	}
}
void textbox_cb(GLUI_Control *control) {
    //printf("Got textbox callback\n");
}

void transform_cb(int mode)
{
	if(mode == 3)
	{
		current_node->x = x_val;
		current_node->y = y_val;
		current_node->z = z_val;
		current_node->degree = rotation_val;		
	}
	else if(mode == 5)
	{
		current_node->transformation_type = curr_trans_index;
	}

}

void select_cb(int control) {
    if(control == 1)
    {
    	int item = gui_node_list->get_current_item();
    	Node* elem = tree_list[item];
    	current_node = elem;
    	update_current();

    }
}

void initialize_dummy_nodes()
{
	Node* node2 = new Node(GEOMETRY);
	Node* node3 = new Node(TRANSFORMATION, "snoopy");
	Node* node4 = new Node(ATTRIBUTE, "garfield");
	Node* node5 = new Node(CAMERA);	
	root->addChild(node2);
	root->addChild(node3);
	node3->addChild(node4);
	node4->addChild(node5);
}

void initialize_kid_nodes()
{
	Node* object = new Node(OBJECT, "Object 1");
	Node* camera = new Node(CAMERA, "Main camera");
	Node* light_node = new Node(LIGHT, "Main light");
	light_node->light = free_lights.back();
	free_lights.pop_back();
	Node* camera_transform = new Node(TRANSFORMATION, "Camera transformer");
	Node* geom_transform = new Node(TRANSFORMATION, "Model transformer");
	Node* geom = new Node(GEOMETRY, "Model 1");
	main_camera = camera;

	root->addChild(object);
	object->addChild(geom_transform);
	geom_transform->addChild(geom);
	root->addChild(camera_transform);
	camera_transform->addChild(camera);
	root->addChild(light_node);
	//geom->setAndUpdateModel("cactus.obj", loader);


}

int main(int argc, char* argv[])
{
	cout << "Please refer to the README for any clarifications. Thanks!" << endl;

	root = new Node(OBJECT, "Root");

	current_node = root;
	lighting_off = false;
	local_coords = true;
	loader = new TrimeshLoader();  
	initialize_kid_nodes();
	float origin[3] = {0,0,0};
	camera_target.assign(origin, origin + 3);
	mode = FACES;
	if(argc >= 3)
	{
		if(!strcmp(argv[1], "-f"))
		{
			loadFromFile(argv[2]);
		}
	}
	else if(argc >= 2)
	{
		Trimesh* mesh = new Trimesh();
		loader->loadOBJ(argv[1], mesh);
		meshes.push_back(mesh);
	}

	glutInit(&argc, argv); //init glut library
	glutInitWindowSize(1200.f, 1000.f);
	glutInitWindowPosition(0,0);

	main_window = glutCreateWindow("Scarlox scene loader");
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutDisplayFunc(myDisplay);
	GLUI_Master.set_glutReshapeFunc(myReshape);
	GLUI_Master.set_glutKeyboardFunc(Keyboard);
	GLUI_Master.set_glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	GLUI_Master.set_glutIdleFunc(idle);
	initGL();

  /****************************************/
  /*         Here's the GLUI code         */
  /****************************************/
	//printf( "GLUI version: %3.2f\n", GLUI_Master.get_version() );

  /*** Create the side subwindow ***/
  glui = GLUI_Master.create_glui_subwindow( main_window, 
					    GLUI_SUBWINDOW_RIGHT );
  /* Panel for adding node */
  create_panel = new GLUI_Panel(glui, "Create node");
	  edit_node_name = new GLUI_EditText(create_panel, "Name: ", add_text);
	  /* Dropdown for adding node */
	  type_selector = new GLUI_Listbox( create_panel, "Type:", &curr_type_string );
	  for(int j = 0; j<7; j++)
	  {
		type_selector->add_item(j, node_type_string[j]);
	  }  
	  new GLUI_Button( create_panel, "Add as child", 0, add_node);
	  new GLUI_Button( create_panel, "Add as parent", 1, add_node);


  /* Panel for editing node */
  	edit_panel = new GLUI_Panel(glui, "Current node");
		geometry_panel = new GLUI_Panel(edit_panel, "Model");
			geometry_panel->disable();
			model_name = new GLUI_EditText(geometry_panel, "Path: ", model_namex);
		transform_panel = new GLUI_Panel(edit_panel, "Transformations");
			transform_panel->disable();
			x_spinner = new GLUI_Spinner(transform_panel, "X:", &x_val, 3, transform_cb);
			y_spinner = new GLUI_Spinner(transform_panel, "Y:", &y_val, 3, transform_cb);
			z_spinner = new GLUI_Spinner(transform_panel, "Z:", &z_val, 3, transform_cb);
			//rotation_spinner = new GLUI_Spinner(transform_panel, "Degree:", &rotation_val, 3, transform_cb);
			transformation_list = new GLUI_Listbox( transform_panel, "      Type:", &curr_trans_index, 5, transform_cb );
			  for(int j = 0; j<4; j++)
			  {
				transformation_list->add_item(j, transformation_type_string[j]);
			  }  
		attribute_panel = new GLUI_Panel(edit_panel, "Attributes");
			attribute_panel->disable();
			attribute_list = new GLUI_Listbox( attribute_panel, "Mode:", &curr_attr, 0, attribute_cb );
			for(int k = 0; k<7; k++)
			{
				attribute_list->add_item(k, attributes[k]);
			} 
			attribute_list->set_int_val(6);
		detail_panel = new GLUI_Panel(edit_panel, "Node details");
			cur_name_text = new GLUI_EditText(detail_panel, "Name: ", cur_name_textx, 0, name_cb);
			cur_type_text = new GLUI_EditText(detail_panel, "Type: ", cur_type_textx);
			cur_id_text = new GLUI_EditText(detail_panel, "ID: ", &cur_id_textx);
			cur_depth_text = new GLUI_EditText(detail_panel, "Depth: ", &cur_depth_textx);
			cur_parent_text = new GLUI_EditText(detail_panel, "Parent: ", cur_parent_textx);
			detail_panel->disable();
		motion_panel = new GLUI_Panel(edit_panel, "Motion");
			radio = new GLUI_RadioGroup(motion_panel, &motion_type, 0, motion_cb);
			motion_panel->disable();
		new GLUI_RadioButton(radio, "Rotation");
		new GLUI_RadioButton(radio, "Translation");
		//new GLUI_Separator(edit_panel);
		GLUI_Panel *tree_panel =  new GLUI_Panel(edit_panel, "Tree management");
		new GLUI_Checkbox( tree_panel, "Hide node: ", &hide_node, 0, hide_cb);
		//new GLUI_Separator(tree_panel);
		new GLUI_Button( tree_panel, "Delete node", 2, add_node); 
		//new GLUI_Separator(tree_panel);
		new GLUI_Button( tree_panel, "Clear tree", 1, control_cb); 


	/* Node selection Panel */
  	obj_panel = new GLUI_Panel(glui, "Node selection");
	root->getTreeText(&tree_list, 0);
	gui_node_list = new GLUI_List(obj_panel, true, 1, select_cb);
	gui_node_list->set_h(300);
	gui_node_list->set_w(150);
	int pd = 0;
	for(int i = 0; i < tree_list.size(); i++)
	{
		string temp = "";
		Node* elem = tree_list[i];		
		if(elem->n_depth != pd)
			counter = 0;
		counter++;		
		for(int j = 0; j < elem->n_depth; j++)
			temp = temp + "   ";
		temp = temp + to_string(counter) + ". ";
		temp = temp + elem->n_name;
		if(elem->isDefaultName)
			temp = temp + " (ID: " + to_string(elem->n_id) + ")"; 
		gui_node_list->add_item(i, temp.c_str());
		pd = elem->n_depth;
	}
	//cout << "tree: " << tree_list.size() << endl;
	for(int n = 0; n < tree_list.size(); n++)
	{
		//cout << "tree: " << tree_list[n]->n_name << endl;
	}
	update_tree_list();

	pthread_t t1;
	pthread_create(&t1, NULL, wait_in, NULL);
	glutMainLoop();	

}

/* Draws a single model */
void draw_model(Trimesh* mesh, int attr)
{

	if(attr == SOLID)
		glDisable(GL_LIGHTING);
	if(mesh != NULL)
	{
		camera_target = mesh->get_target();
		glTranslatef(-camera_target[0], -camera_target[1], -camera_target[2]);
		mesh->drawFaces(attr);
		if(attr == 5)
			mesh->drawVNormals();
		if(attr == 4)
			mesh->drawFNormals();


	}	
}

void apply_transformation(Node* node)
{
	//vector<float> ctarget = node->model->get_target();
	//cout << "transformation applied" << endl;
	if(node->type == TRANSFORMATION || node->type == OBJECT)
	{
		switch(node->transformation_type)
		{
			case ROTATE:
				//cout << "rotating by " << node->x <<  endl;
				glRotatef(node->x, 0, 1, 0);
				glRotatef(node->y, 0, 0, 1);
				glRotatef(node->z, 1, 0, 0);
				break;
			case TRANSLATE:
				glTranslatef(node->x, node->y, node->z);
				break;
			case SCALE:
				glScalef(node->x, node->y, node->z);
				break;
		}
	}
	else
	{
		switch(node->motion_type)
		{
			case MOTION_ROTATE:
				glRotatef(node->x * node->tick, 0, 1, 0);
				glRotatef(node->y * node->tick, 0, 0, 1);
				glRotatef(node->z * node->tick, 1, 0, 0);
			break;

			case MOTION_TRANSLATE:
				glTranslatef(node->x * node->tick, node->y*node->tick, node->z * node->tick);
			break;

		}


	}

}

void preprocess_camera_rotation()
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 1.0f;
	Node *parent;
	if(main_camera)
	 parent = main_camera->parent;
	vector<Node*> ancestors;
	while(parent)
	{
		if(parent->type == TRANSFORMATION)
			ancestors.push_back(parent);
		parent = parent->parent;
	}
	for(int i = 0; i < ancestors.size(); i++)
	{
		if(ancestors[i]->transformation_type == ROTATE)
		{
			glRotatef(ancestors[i]->x, 0, 1, 0);
			glRotatef(ancestors[i]->y, 1, 0, 0);
		}	

	}

}

void draw_axes( float scale )
{
  //glDisable( GL_LIGHTING );

  glPushMatrix();
  
  glScalef( scale, scale, scale );
  glTranslatef(0, 0, 0);
  preprocess_camera_rotation();
  glBegin( GL_LINES );
  
  glColor3f( 1.0, 0.0, 0.0 );
  glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 ); /* Letter X*/
  glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 ); /* X axis */

  glColor3f( 0.0, 1.0, 0.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 ); /* Y axis */

  glColor3f( 0.0, 0.0, 1.0 );
  glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 ); /* Z axis  */
  glEnd();

  glPopMatrix();

  //glEnable( GL_LIGHTING );
}

/* Helper function to recursively processes all the nodes */
void process_nodes(Node* node, int attr)
{
	int mode = attr;
	if(!node->hide)
	{
		switch(node->type)
		{
			case OBJECT:

				apply_transformation(node);
				break;

			case GEOMETRY:
				if(node->model)
					draw_model(node->model, mode);
				break;

			case TRANSFORMATION:
				apply_transformation(node);


				break;

			case ATTRIBUTE:
				mode = node->attr_type;
				break;

			case LIGHT:
				glEnable(node->light);
				//cout << "light" << node->light << endl;
				break;

			case CAMERA:
					
				break;

			case MOTION:

				apply_transformation(node);

				break;

			default:
				cout << "Incorrect node type" << endl;
		}		
	}
	node->tick++;
	for(int i = 0; i < node->children.size(); i++)
	{
		glPushMatrix();
		process_nodes(node->children[i], mode);
		glPopMatrix();
	}
}



void preprocess_camera()
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 3.0f;
	Node *parent;
	if(main_camera)
	 parent = main_camera->parent;
	vector<Node*> ancestors;
	while(parent)
	{
		if(parent->type == TRANSFORMATION)
			ancestors.push_back(parent);
		parent = parent->parent;
	}
	
	for(int i = 0; i < ancestors.size(); i++)
	{
		if(ancestors[i]->transformation_type == TRANSLATE)
		{
			glTranslatef(-ancestors[i]->x, -ancestors[i]->y, -ancestors[i]->z);
		}
		else if(ancestors[i]->transformation_type == ROTATE)
		{
			/* Need atomic operations */
			float temp_x = x;
			float temp_y = y;
			float temp_z = z;
			temp_x = x * cos(ancestors[i]->x * PI / 180) + z * sin(ancestors[i]->x * PI / 180);
			temp_z = z * cos(ancestors[i]->x * PI / 180) - x * sin(ancestors[i]->x * PI / 180);	
			x = temp_x;
			z = temp_z;
			temp_y = y * cos(ancestors[i]->y * PI / 180) - z * sin(ancestors[i]->y * PI / 180);
			temp_z = z * cos(ancestors[i]->y * PI / 180) + y * sin(ancestors[i]->y * PI / 180);	
			y = temp_y;
			z = temp_z;	
			glRotatef(ancestors[i]->x, 0, 1, 0);
			glRotatef(ancestors[i]->y, 1, 0, 0);
		}	

	}
	glTranslatef(0,0,-3.f);
	gluLookAt(0, 0, 3 - 1.0f, 
			0, 0, -0.0f,
			0.0, 1.0, 0.0);
/*	gluLookAt(0, 0, 0, 
			0, 0, 0,
			0.0, 1.0, 0.0);*/

}

void process_nodes()
{
	//for(int i = 0; i < 8; i++)
	//	glDisable(GL_LIGHT0 + i);

	glDisable(GL_LIGHTING);

	draw_axes(0.1f);
	preprocess_camera();
/*	gluLookAt(0.0f, 0, 0.0f, 
			1.0f, 0.5f, -1.0f,
			0.0, 1.0, 0.0);*/
	
	//glPushMatrix();
	glEnable(GL_LIGHTING);		
	process_nodes(root, SHADED);
}



void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	myReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(main_camera->x != 0 | main_camera->y != 0 | main_camera->z != 0 )
	{
		//cout << "TF" << endl;
		glFrustum(main_camera->x, 
			main_camera->x + main_camera->degree,
			main_camera->y,
			main_camera->y + main_camera->degree,
			main_camera->z,
			main_camera->z+ main_camera->degree);
	}
	if(!lighting_off)
	{
		float ambient[] = {0.0f, 0.0f, 0.0f, 0.0f};
		glEnable(GL_LIGHTING);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		//glEnable(GL_LIGHT0);
		//for(int i = 0;)
		//glDisable(GL_LIGHT0);
		for(int i = 0; i < all_lights.size(); i++)
		{
			glDisable(all_lights[i]);
		}
		glEnable(GL_COLOR_MATERIAL);		
	}
	process_nodes();	
	//if(!lighting_off)
		//glDisable(GL_LIGHTING);
	glutSwapBuffers();
	glFlush();
	usleep(30000);
	glutPostRedisplay();
}

void flushTransformations()
{
	Perm_x_rotate = 0;
	Perm_y_rotate = 0;
	x_rotate = 0;
	y_rotate = 0;
	perm_zoom = 0;
	zoom = 0;
	perm_x_translate = 0;
	perm_y_translate = 0;
	x_translate = 0;
	y_translate = 0;
}

void parseInput(char cmd[LINE_SIZE], bool isCommand)
{
    std::vector<char*> v;

    char* chars_array = strtok(cmd, " ");
    while(chars_array)
    {
        v.push_back(chars_array);
        chars_array = strtok(NULL, " ");
    }
    float span;
    if(meshes.size() > 0)
    {
	    float diam = meshes.back()->get_diam();
	    float z = 1.0f + diam * 4.0f + perm_zoom + zoom;
	    float theta = 30.0f/2.0f;
	    float tang = tan(theta);
	    float opposite = tang * z;
	    span = -opposite;    	
    }
	int i;
	Trimesh* mesh;
	if(v.size() == 0)
		return;
	if(meshes.size() > 0)
		mesh = meshes.back();
	if(!strcmp(v[0], "L") & v.size() >= 2)
	{
		cout << "Loading file: " << v[1]  << endl;
		Trimesh* mesh = new Trimesh();

		if(loader->loadOBJ(v[1], mesh))
		{
			meshes.push_back(mesh);
			flushTransformations();
		}		
	}
	else if(!strcmp(v[0], "D"))
	{
		if(v.size() >= 2)
		{
			meshes.erase(meshes.begin() + atoi(v[1]));
			printFiles();
		}	
		else
			meshes.pop_back();
	}
	else if(!strcmp(v[0], "I"))
	{
		mesh->addTransformation(4.0f, 0.f, 0.f, 0.f, 0.f);
		flushTransformations();
	}
	else if(!strcmp(v[0], "T") && v.size() >= 4)
	{
		if(local_coords && false)
			mesh->addTransformation(1.0f, 0.f, span * atof(v[1]), -span * atof(v[2]), span * atof(v[3]));
		else
			mesh->addTransformation(1.0f, 0.f, atof(v[1]), -atof(v[2]), atof(v[3]));
	}
	else if(!strcmp(v[0], "S") && v.size() >= 4)
	{
		if(local_coords && false)
		{

			mesh->addTransformation(2.0f, 0.f, 
			 atof(v[1]) == 1.0f ? 1.0f : span * atof(v[1]),
			 atof(v[2]) == 1.0f ? 1.0f : span * atof(v[2]), 
			 atof(v[3]) == 1.0f ? 1.0f : span * atof(v[3]));
		}		
		else
			mesh->addTransformation(2.0f, 0.f, atof(v[1]), atof(v[2]), atof(v[3]));
	}
	else if(!strcmp(v[0], "R") && v.size() >= 5)
	{
		mesh->addTransformation(3.0f, atof(v[1]), atof(v[2]), atof(v[3]), atof(v[4]));
	}
	else if(!strcmp(v[0], "V"))
	{
		local_coords = true;
	}
	else if(!strcmp(v[0], "W"))
	{
		local_coords = false;
	}
	else if(!strcmp(v[0], "POINT"))
	{
		mode = POINTS;
	}
	else if(!strcmp(v[0], "WIRE"))
	{
		mode = LINES;
	}
	else if(!strcmp(v[0], "SOLID"))
	{
		mode = FACES;
		lighting_off = true;
	}
	else if(!strcmp(v[0], "SHADED"))
	{
		mode = FACES;
		lighting_off = false;
	}
	else if(!strcmp(v[0], "FNORMALS"))
	{
		fnormals = !fnormals;
	}
	else if(!strcmp(v[0], "VNORMALS"))
	{
		vnormals = !vnormals;
	}
	else if(!strcmp(v[0], "QUIT"))
	{
		exit(0);
	}
	else if(!strcmp(v[0], "ROTATOPOTATO"))
	{
		auto_rotate_enabled = !auto_rotate_enabled;
	}
	else if(!strcmp(v[0], "OBJECTS"))
	{
		printFiles();
	}
	else if(!strcmp(v[0], "SWITCH"))
	{
		if(v.size() >= 2)
		{
			int ind = atoi(v[1]);
			meshes.push_back(meshes[ind]);
			meshes.erase(meshes.begin() + ind);
			printFiles();
		}
	}
	else if(!strcmp(v[0], "DRAW"))
	{
		if(v.size() >= 2)
		{
			int ind = atoi(v[1]);
			if(meshes.size()-1 >= ind)
				meshes[ind]->draw = true;
			printFiles();
		}
	}
	if(isCommand)
		newCommand = false;
}

bool loadFromFile(char* fileName)
{
	ifstream ifs;
	char line[LINE_SIZE];
	ifs.open(fileName);
	bool not_screwy = ifs.good();
	while(!ifs.eof() && not_screwy)
	{
		ifs.getline(line,LINE_SIZE);
		char data[LINE_SIZE];
		strcpy(data, line);
		data[strlen(line)-1] = '\0';
		parseInput(data, false);
	}
	ifs.close();
	return not_screwy;
}



void idle()
{
	ready = true;
	glui->sync_live();
	current_node->setAndUpdateModel(model_namex.c_str(), loader);



	//cout << "wtf" << cur_name_textx << endl;
}

void printFiles()
{
	cout << endl;
	cout << "-----Stored objects-----" << endl;
	int i;
	for(i = 0; i < meshes.size(); i++)
	{
		cout << "[" << i << "]: " << meshes[i]->name;
		if(meshes[i]->draw)
			cout << " <- (DRAW)";		
		if(i == meshes.size()-1)
			cout << " <- (ACTIVE)";
		cout << endl;
	}
	cout << endl;

}


void myReshape(GLsizei width, GLsizei height)
{
   // Compute aspect ratio of the new window
   if (height == 0) height = 1;                // To prevent divide by 0
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
   // Set the viewport to cover the new window
   glViewport(0, 0, width, height);
 
   // Set the aspect ratio of the clipping volume to match the viewport
   
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
    glLoadIdentity();             // Reset
	gluPerspective(70.0f, aspect, 1.0f, 20.0f);
	

	
	

}


void initGL()
{
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glClearDepth(100.0f);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void MouseButton(int button, int state, int x, int y) {
  // Respond to mouse button presses.
  // If button1 pressed, mark this state so we know in motion function.
	

	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		perm_zoom += zoom;
		zoom = 0;
		mousePressed = true;
		x_click = x;
		y_click = y;
	}
	else if(button == GLUT_LEFT_BUTTON)
	{
		 mousePressed = false;

		 Perm_x_rotate += x_rotate;
		 Perm_y_rotate +=  y_rotate;
		 x_rotate = 0;
		 y_rotate = 0;
		 x_click = 0;
		 y_click = 0;
	}
	else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{

		rightPressed = true;
		x_click = x;
		y_click = y;
	}
	else if(button == GLUT_RIGHT_BUTTON)
	{
		 rightPressed = false;

		 perm_zoom += zoom;
		 zoom = 0;
		 x_click = 0;
		 y_click = 0;
	}
	else if(button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{

		middlePressed = true;
		x_click = x;
		y_click = y;
	}
	else if(button == GLUT_MIDDLE_BUTTON)
	{
		 middlePressed = false;
		 perm_x_translate += x_translate;
		 perm_y_translate += y_translate;
		 x_translate = 0;
		 y_translate = 0;
		 x_click = 0;
		 y_click = 0;
	}
	//cout << "BUTTON: " << button << endl;
}

void MouseMotion(int x, int y) {
  // If button1 pressed, zoom in/out if mouse is moved up/down.
	if(x_click == 0 && y_click == 0)
		return;
  if (mousePressed)
    {
      x_rotate = y - y_click;
	  y_rotate = x - x_click;
	  //cout << "xrotate: " << x_rotate << endl;
    }
  if(rightPressed)
  {
	  zoom = ((float)(y - y_click))/100.0f;
  }
  if(middlePressed)
  {
	  x_translate = x - x_click;
	  y_translate = y - y_click;
  }

	//glutPostRedisplay();
}

enum {
  MENU_LIGHTING = 1,
  MENU_POLYMODE,
  MENU_PERSPECTIVE,
  MENU_AUTO_ROTATE,
  MENU_TEXTURING,
  MENU_INCREASE_ROTATE_SPEED,
  MENU_INCREASE_AUTO_ROTATE_SPEED,
  MENU_DECREASE_ROTATE_SPEED,
  MENU_DECREASE_AUTO_ROTATE_SPEED,
  MENU_LEVEL_0,
  MENU_LEVEL_1,
  MENU_LEVEL_2,
  MENU_LEVEL_3,
  MENU_LEVEL_4,
  MENU_LEVEL_5,
  MENU_EXIT
};

void SelectFromMenu(int idCommand) {
  switch (idCommand)
    {
		 case MENU_PERSPECTIVE:
			perspective_mode_enabled = !perspective_mode_enabled;
			break;
		 case MENU_LIGHTING:
			lighting_enabled = !lighting_enabled;
			break;
		 case MENU_AUTO_ROTATE:
			 auto_rotate_enabled = !auto_rotate_enabled;
			 break;
		 case MENU_INCREASE_ROTATE_SPEED:
			 rotate_speed++;
			 break;
		 case MENU_INCREASE_AUTO_ROTATE_SPEED:
			 auto_rotate_speed++;
			 break;
		 case MENU_DECREASE_ROTATE_SPEED:
			 rotate_speed--;
			 break;
		 case MENU_DECREASE_AUTO_ROTATE_SPEED:
			 auto_rotate_speed--;
			 break;
    }
  // Almost any menu selection requires a redraw
  //glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key)
  {
  case 27:             // ESCAPE key
	  exit (0);
	  break;
  case 'l':
	  SelectFromMenu(MENU_LIGHTING);
	  break;
  case 'r':
	  SelectFromMenu(MENU_AUTO_ROTATE);
	  break;
  case 'p':
	  SelectFromMenu(MENU_PERSPECTIVE);
	  break;
  case 't':
	  SelectFromMenu(MENU_TEXTURING);
	  break;
  case '+':
	  SelectFromMenu(MENU_INCREASE_AUTO_ROTATE_SPEED);
	  break;
  case '-':
	  SelectFromMenu(MENU_DECREASE_AUTO_ROTATE_SPEED);
	  break;
  case '0':
	  SelectFromMenu(MENU_LEVEL_0);
	  break;
	case '1':
	  SelectFromMenu(MENU_LEVEL_1);
	  break;
	case '2':
	  SelectFromMenu(MENU_LEVEL_2);
	  break;
	case '3':
	  SelectFromMenu(MENU_LEVEL_3);
	  break;
	case '4':
	  SelectFromMenu(MENU_LEVEL_4);
	  break;
	case '5':
	  SelectFromMenu(MENU_LEVEL_5);
	  break;
	}
}





