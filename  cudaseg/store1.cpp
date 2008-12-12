
#include "CIPS.h"

#define  SHEIGHT 20
#define  SWIDTH  15




int is_in_image(il, ie, height, width){
   int il, ie;
   long height, width;

   int result = 1;

   if(il < 0){
      printf("\nil=%d tool small", il);
      result = 0;
   }

   if(ie < 0){
      printf("\nie=%d tool small", ie);
      result = 0;
   }

   if((il+SHEIGHT) > height ){
      printf("\nll=%d tool big", il+SHEIGHT);
      result = 0;
   }

   if((ie+SWIDTH) > width ){
      printf("\nle=%d tool big", ie+SWIDTH);
      result = 0;
   }

   return(result);


}  /* ends is_in_image */




int show_screen(the_image, il, ie){
   int  il, ie;
   short **the_image;

   int i, j;

   printf("\n     ");
   for(i=ie-1; i<ie-1+SWIDTH; i++)
      printf("-%3d", i);

   for(i=il-1; i<il-1+SHEIGHT; i++){
      printf("\n%4d>", i);
      for(j=ie-1; j<ie-1+SWIDTH; j++){
         printf("-%3d", the_image[i][j]);
      }
   }

}  /* ends show_screen */



short **allocate_image_array(length, width){
   long  length, width;

   int i;
   short **the_array;

   the_array = malloc(length * sizeof(short  *));
   for(i=0; i<length; i++){
      the_array[i] = malloc(width * sizeof(short ));
      if(the_array[i] == '\0'){
         printf("\n\tmalloc of the_image[%d] failed", i);
      }  /* ends if */
   }  /* ends loop over i */
   return(the_array);

} 
does_not_exist(file_name){
    char file_name[];

   FILE *image_file;
   int  result;

   result = 1;
   image_file = fopen(file_name, "rb");
   if(image_file != NULL){
      result = 0;
      fclose(image_file);
   }
   return(result);
}  /* ends does_not_exist */




int get_image_size(file_name, rows, cols){
   char *file_name;
   long *cols, *rows;

   int is_bmp  = 0,
       is_tiff = 0,
       result  = 0;
   struct bitmapheader bmph;
   struct tiff_header_struct tiffh;

   if(is_a_bmp(file_name)){
      is_bmp = 1;
      read_bm_header(file_name, &bmph);
      *rows = bmph.height;
      *cols = bmph.width;
   }  /* ends if is_a_bmp */

   if(is_a_tiff(file_name)){
      is_tiff = 1;
      read_tiff_header(file_name, &tiffh);
      *rows = tiffh.image_length;
      *cols = tiffh.image_width;
   }  /* ends if is_a_bmp */

   if(is_bmp == 1  ||  is_tiff == 1)
      result = 1;

   return(result);
}  /* ends get_image_size */






short **allocate_image_array(length, width){
   long  length, width;

   int i;
   short **the_array;

   the_array = malloc(length * sizeof(short  *));
   for(i=0; i<length; i++){
      the_array[i] = malloc(width * sizeof(short ));
      if(the_array[i] == '\0'){
         printf("\n\tmalloc of the_image[%d] failed", i);
      }  /* ends if */
   }  /* ends loop over i */
   return(the_array);

}  /* ends allocate_image_array */




read_bmp_image(file_name, array){
   char  *file_name;
   short **array;

   FILE   *fp;
   int    i, j;
   int    negative = 0,
          pad      = 0,
          place    = 0;
   long   colors   = 0,
          height   = 0,
          position = 0,
          width    = 0;
   struct bmpfileheader file_header;
   struct bitmapheader  bmheader;
   struct ctstruct rgb[GRAY_LEVELS+1];
   unsigned char uc;

   read_bmp_file_header(file_name, &file_header);
   read_bm_header(file_name, &bmheader);
   if(bmheader.bitsperpixel != 8){
      printf("\nCannot read image when bits per"
      "pixel is not 8");
      exit(1);
   }

   if(bmheader.colorsused == 0)
      colors = GRAY_LEVELS + 1;
   else
      colors = bmheader.colorsused;
   read_color_table(file_name, &rgb, colors);

   fp = fopen(file_name, "rb");
   fseek(fp, file_header.bitmapoffset, SEEK_SET);

   width = bmheader.width;
   if(bmheader.height < 0){
      height   = bmheader.height * (-1);
      negative = 1;
   }
   else
      height = bmheader.height;

   pad = calculate_pad(width);

   for(i=0; i<height; i++){
      for(j=0; j<width; j++){
         place = fgetc(fp);
         uc = (place & 0xff);
         place = uc;
         array[i][j] = rgb[place].blue;
      }  /* ends loop over j */
      if(pad != 0){
         position = fseek(fp, pad, SEEK_CUR);
      }  /* ends if pad 1= 0 */
   }  /* ends loop over i */

   if(negative == 0)
      flip_image_array(array, height, width);

}  /* ends read_bmp_image */






int free_image_array(the_array, length){
   short **the_array;
   long  length;

   int i;
   for(i=0; i<length; i++)
      free(the_array[i]);
   return(1);
}  /* ends free_image_array */
















int main(argc, argv){
   int argc;
   char *argv[];

   char  in_name[MAX_NAME_LENGTH];
   char  response[MAX_NAME_LENGTH];
   int   ie, il, not_done, temp_ie, temp_il;
   long  height, width;
   short **the_image;
   
      /******************************************
      *
      *   Ensure the command line is correct.
      *
      ******************************************/

   if(argc != 4){
    printf("\nusage: showi input-image il ie");
    exit(0);
   }

   strcpy(in_name,  argv[1]);
   il = atoi(argv[2]);
   ie = atoi(argv[3]);
   
      /******************************************
      *
      *   Ensure the input image exists.
      *   Allocate an image array.
      *   Read the image and show it on the 
      *   screen.
      *
      ******************************************/

   if(does_not_exist(in_name)){
      printf("\nERROR input file %s does not exist",
             in_name);
      exit(0);
   }  /* ends if does_not_exist */

   get_image_size(in_name, &height, &width);
   the_image = allocate_image_array(height, width);
   read_image_array(in_name, the_image);

   temp_il  = il;
   temp_ie  = ie;
   not_done = 1;

   while(not_done){
      if(is_in_image(temp_il, temp_ie, height, width)){
         il = temp_il;
         ie = temp_ie;
         show_screen(the_image, il, ie);
      }  /* ends if is_in_image */

      printf("\n\n x=quit j=down k=up h=left l=right"
             "\nEnter choice and press Enter:  ");
      gets(response);

      if(response[0] == 'x' || response[0] == 'X')
         not_done = 0;
      if(response[0] == 'j' || response[0] == 'J')
         temp_il = temp_il + ((3*SHEIGHT)/4);
      if(response[0] == 'k' || response[0] == 'K')
         temp_il = temp_il - ((3*SHEIGHT)/4);
      if(response[0] == 'h' || response[0] == 'H')
         temp_ie = temp_ie - ((3*SWIDTH)/4);
      if(response[0] == 'l' || response[0] == 'L')
         temp_ie = temp_ie + ((3*SWIDTH)/4);
   }  /* ends while not_done */

   free_image_array(the_image, height);

}  /* ends main */



