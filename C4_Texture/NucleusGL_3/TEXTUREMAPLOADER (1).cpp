***********************************************************************************************************************************************
REPLACE THE CURRENT read_polys FUNCTION WITH THIS NEW ONE
JALLAN 19/03/02 
R.J.Cant 13/03/03
***********************************************************************************************************************************************

int read_polys(FILE *infile,POLYGON *plist)
{
	char instring[80];
   FILE *texfile;
   int i,tex_n,max_tex_n=0;
   TEXTURE_MAP *tex_locations[100];
	int np=0;
	int r,g,b;
   float len;
	debugfile=fopen("debug.txt","w");

   fgets(instring,80,infile);
   while (strncmp(instring,"end of texture",14)!=0)
   {
      for (i=0;i<80;i++)if (instring[i]=='\n')instring[i]=0;
      texfile=fopen(instring,"rb");
      tex_locations[max_tex_n]=(TEXTURE_MAP *)malloc(sizeof(TEXTURE_MAP));
      fgets(instring,80,infile);//get sizes
      sscanf(instring,"%d%d",&(tex_locations[max_tex_n]->sx),&(tex_locations[max_tex_n]->sy));   
      //allocate memory for  texture
//      printf("texture size %dx%d\n",tex_locations[max_tex_n]->sx,tex_locations[max_tex_n]->sy);
      tex_locations[max_tex_n]->map=(char *)malloc(sizeof(char)*3*tex_locations[max_tex_n]->sx*tex_locations[max_tex_n]->sy);
      //read file - leaving in packed format for now
      fread(tex_locations[max_tex_n]->map,sizeof(char),3*tex_locations[max_tex_n]->sx*tex_locations[max_tex_n]->sy,texfile);
      //printf("texture %d stored at %X \n",max_tex_n,tex_locations[max_tex_n]->map);
      max_tex_n++;
      fclose(texfile);
      fgets(instring,80,infile);//next texture
   }
	do
	{
    fgets(instring,80,infile);       //Read  first/next line of file
    sscanf(instring,"%d",&plist[np].nv);   // get number of vertices
    fprintf(debugfile," n verts %d\n",plist[np].nv);   // get number of vertices
		if  (plist[np].nv==0) break;//get out if terminating zero found
		for (i=0;i<plist[np].nv;i++)
		{
      fgets(instring,80,infile);          //Read  next line of file
         sscanf(instring,"%f%f%f%f%f",&(plist[np].vert[i].x),&(plist[np].vert[i].y),&(plist[np].vert[i].z),&(plist[np].tpos[i].x),&(plist[np].tpos[i].y));
                                          //Get Coordinates
		}
    //Create  Face normal vector
    plist[np].normal=cross(vector_dif(plist[np].vert[0],plist[np].vert[1]),vector_dif(plist[np].vert[0],plist[np].vert[2]));
    len=sqrt(dot(plist[np].normal,plist[np].normal));//Calculate length of vector

    plist[np].normal.x/=len;//Normalise
    plist[np].normal.y/=len;//each
    plist[np].normal.z/=len;//component
    fgets(instring,80,infile);  //Read  next line of file
    sscanf(instring,"%d%d%d%d",&r,&g,&b,&tex_n); // Get Colour, texture
    plist[np].colour=b|(g<<8)|(r<<16);//Organise colours in word
    if (tex_n<max_tex_n)   plist[np].tmap=tex_locations[tex_n]; //copy texture pointer
    else  plist[np].tmap=tex_locations[0];//safety measure - go for first texture
		np++;
  } while(1);
  return np;  //Return number of vertices
}