#include "mainwindow.h"

int current_page = 0;

void free_array_with_alloced(void **list, const int size)
{
    if(list != NULL)
    {
        for(int i = 0; i < size; ++i)
        {
            if(list[i] != NULL)
            {
                free(list[i]);
                list[i] = NULL;
            }
        }

        free(list);
        list = NULL;

        printf("Free Success\n");
    }

}

int get_image_file_count(struct dirent **src, const int size, int *dst)
{
    int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
    memset(number_list, 0, sizeof(int) * LIST_BUFFER);

    int count = 1;
    for(int i = 0; i < size; ++i)
    {
        if((strstr(src[i]->d_name, ".png") != NULL || strstr(src[i]->d_name, ".jpg") != NULL) 
                && count < LIST_BUFFER)
        {
            number_list[count - 1] = i;
            count++;
            int *tmp = (int*)realloc(number_list, sizeof(int) * count);
            if(tmp != NULL)
            {
                number_list = tmp;
            }
        }
    }

    int *tmp = (int*)realloc(dst, sizeof(int) * count);
    if(tmp != NULL)
    {
        dst = tmp;
        memset(dst, 0, sizeof(int) * count);
        memcpy(dst, number_list, sizeof(int) * count);
    }
    else
    {
        free(tmp);
    }


    free(number_list);

    return count - 1;
}

int create_image_list(char **image_list)
{
    struct dirent **file_list;

    int r = scandir("./", &file_list, NULL, alphasort);

    int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
    memset(number_list, 0, sizeof(int) * LIST_BUFFER);

    int count = get_image_file_count(file_list, r, number_list);

    printf("count: %d, r: %d\n", count, r);

    if(count < LIST_BUFFER)
    {
        size_t image_list_size = sizeof(char*) * count;
        char **tmp = (char**)realloc(image_list, image_list_size);

        if(tmp != NULL)
        {
            image_list = tmp;

            // memset(image_list, 0, image_list_size);

            for(int i = 0; i < count; ++i)
            {
                const int target = number_list[i];
                const size_t length = strlen(file_list[target]->d_name);
                printf("%s\n", file_list[target]->d_name);

                image_list[i] = (char*)calloc(length + 1, 1);

                strncpy(image_list[i], file_list[target]->d_name, length);
            }
        }
        else
        {
            free(tmp);
        }
    }

    free(number_list);

    free_array_with_alloced((void**)file_list, r);

    return count;
}

void set_image_list()
{
    detail = (DirectoryDetail_t*)malloc(sizeof(DirectoryDetail_t));
    if(detail != NULL)
    {
        memset(detail, 0, sizeof(DirectoryDetail_t));
        detail->image_list = (char**)malloc(LIST_BUFFER);
        if(detail->image_list != NULL)
        {
            printf("Not NULL from detail->image_list\n");
            detail->image_count = create_image_list(detail->image_list);
        }
        else
        {
            printf("Failed malloc to detail->image_list\n");
            free(detail->image_list);
        }
    }

    /*
       if(detail->image_list != NULL) 
       {
       for(int i = 0; i < detail->image_count; i++)
       {
       if(detail->image_list[i] != NULL)
       {
       printf("%s\n", detail->image_list[i]);
       }
       }
       free_array_with_alloced((void**)detail->image_list, detail->image_count);
       }
       else
       {
       free(detail->image_list);
       }
       */

}


gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch(event->keyval) {

        case GDK_KEY_Right:
            current_page++;
            if(current_page >= detail->image_count)
            {
                current_page = 0;
            }

            printf("%s\n", detail->image_list[current_page]);

            set_image_container(current_page);

            gtk_image_clear((GtkImage*)image);

            gtk_image_set_from_pixbuf((GtkImage*)image, image_container_list[current_page]->dst);

            printf("pressed right arrow key\n");

            return TRUE;

        case GDK_KEY_Left:
            current_page--;
            if(current_page < 0)
            {
                current_page = detail->image_count - 1;
            }

            set_image_container(current_page);

            gtk_image_clear((GtkImage*)image);

            gtk_image_set_from_pixbuf((GtkImage*)image, image_container_list[current_page]->dst);

            printf("pressed right left key\n");

            return TRUE;

        default:
            break;
    }

    return FALSE;
}

void Close()
{
    if(detail->image_list != NULL) 
    {                                                                                
        free_array_with_alloced((void**)detail->image_list, detail->image_count);    
    }                                                                                
    else                                                                             
    {                                                                                
        free(detail->image_list);                                                    
    }                                                                                


    if(image_container_list != NULL)
    {
        free_array_with_alloced((void**)image_container_list, detail->image_count);
    }
    else
    {
        free(image_container_list);
    }

    free(detail);

    printf("Closed\n");
}


void set_image_container(int position)
{
    if(image_container_list[position] == NULL)
    {
        image_container_list[position] = malloc(sizeof(Image_Container_List_t));
        memset(image_container_list[position], 0, sizeof(Image_Container_List_t));

        image_container_list[position]->err = NULL;
        image_container_list[position]->src = gdk_pixbuf_new_from_file(detail->image_list[current_page], &image_container_list[position]->err);
        image_container_list[position]->src_height = gdk_pixbuf_get_height(image_container_list[position]->src);
        image_container_list[position]->src_width = gdk_pixbuf_get_width(image_container_list[position]->src);


        int width = image_container_list[position]->src_width;
        int height = image_container_list[position]->src_height;

        int *asperct_raito = calc_aspect_raito(width, height, mygcd(width, height));
        float w_aspect = (double)asperct_raito[0];
        float h_aspect = (double)asperct_raito[1];
        free(asperct_raito);

        if(height > window.height)
        {
          int diff = height - window.height;
          height = height - diff;
          int result = (int)ceil((double)height * (w_aspect / h_aspect));
          printf("%d\n", result);
          width = result;
        }
        
        image_container_list[position]->dst = gdk_pixbuf_scale_simple(image_container_list[position]->src, width, height, GDK_INTERP_BILINEAR);
    }

}
