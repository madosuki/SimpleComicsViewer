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
        if(count < LIST_BUFFER)
        {
            FILE *file = fopen(src[i]->d_name, "rb");
            if(file != NULL)
            {
                fseek(file, 0, SEEK_END);
                long file_length = ftell(file);
                rewind(file);

                printf("%s, File Size: %ld\n", src[i]->d_name, file_length);

                char *buffer;
                if(file_length > 8)
                {
                    buffer = (char*)calloc(8, 1);
                    fread(buffer, 1, png_sig_size, file);

                    int isImageFile = 0;
                    int sig_count = 0;
                    if(buffer[0] == (char)137)
                    {
                        for(int j = 0; j < png_sig_size; ++j)
                        {
                            if(buffer[j] == (char)png_sig[j])
                            {
                                sig_count++;
                            }
                        }
                    }
                    else if(buffer[0] == (char)255)
                    {

                        for(int j = 0; j < jpg_sig_size; ++j)
                        {
                            if(buffer[j] == (char)jpg_sig[j])
                            {
                                sig_count++;
                            }
                        }
                    }

                    free(buffer);

                    switch(sig_count)
                    {
                        case 8:
                            isImageFile = 1;
                            break;
                        case 4:
                            isImageFile = 2;
                            break;
                        default:
                            break;
                    }

                    if(isImageFile == 1 || isImageFile == 2)
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

                fclose(file);
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

int create_image_path_list(char **image_path_list)
{
    struct dirent **file_list;

    int r = scandir("./", &file_list, NULL, alphasort);

    int *number_list = (int*)malloc(sizeof(int) * LIST_BUFFER);
    memset(number_list, 0, sizeof(int) * LIST_BUFFER);

    int count = get_image_file_count(file_list, r, number_list);

    printf("count: %d, r: %d\n", count, r);

    if(count < LIST_BUFFER)
    {
        size_t image_path_list_size = sizeof(char*) * count;
        char **tmp = (char**)realloc(image_path_list, image_path_list_size);

        if(tmp != NULL)
        {
            image_path_list = tmp;

            // memset(image_list, 0, image_list_size);

            for(int i = 0; i < count; ++i)
            {
                const int target = number_list[i];
                const size_t length = strlen(file_list[target]->d_name);
                printf("%s\n", file_list[target]->d_name);

                image_path_list[i] = (char*)calloc(length + 1, 1);

                strncpy(image_path_list[i], file_list[target]->d_name, length);
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

void set_image_path_list()
{
    detail = (DirectoryDetail_t*)malloc(sizeof(DirectoryDetail_t));
    if(detail != NULL)
    {
        memset(detail, 0, sizeof(DirectoryDetail_t));
        detail->image_path_list = (char**)malloc(LIST_BUFFER);
        if(detail->image_path_list != NULL)
        {
            printf("Not NULL from detail->image_list\n");
            detail->image_count = create_image_path_list(detail->image_path_list);
        }
        else
        {
            printf("Failed malloc to detail->image_list\n");
            free(detail->image_path_list);
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

            printf("%s\n", detail->image_path_list[current_page]);

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
    if(detail->image_path_list != NULL) 
    {                                                                                
        free_array_with_alloced((void**)detail->image_path_list, detail->image_count);    
    }                                                                                
    else                                                                             
    {                                                                                
        free(detail->image_path_list);                                                    
    }                                                                                


    if(image_container_list != NULL)
    {
        for(int i = 0; i < detail->image_count; i++)
        {
            if(image_container_list[i] != NULL)
            {
                free(image_container_list[i]->aspect_raito);
                image_container_list[i]->aspect_raito = NULL;

                if(image_container_list[i]->src != NULL)
                {
                    g_object_unref(image_container_list[i]->src);
                }
            }
        }

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
        image_container_list[position] = malloc(sizeof(Image_Container_t));
        memset(image_container_list[position], 0, sizeof(Image_Container_t));

        image_container_list[position]->err = NULL;
        image_container_list[position]->src = gdk_pixbuf_new_from_file(detail->image_path_list[current_page], &image_container_list[position]->err);
        image_container_list[position]->src_height = gdk_pixbuf_get_height(image_container_list[position]->src);
        image_container_list[position]->src_width = gdk_pixbuf_get_width(image_container_list[position]->src);

        int width = image_container_list[position]->src_width;
        int height = image_container_list[position]->src_height;

        image_container_list[position]->aspect_raito = calc_aspect_raito(width, height, mygcd(width, height));

    }

    update_image_size(position);

}

void update_image_size(int position)
{
    gint window_width = 0;
    gint window_height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
    window.width = window_width;
    window.height = window_height;

    int width = image_container_list[position]->src_width;
    int height = image_container_list[position]->src_height;

    double w_aspect = (double)image_container_list[position]->aspect_raito[0];
    double h_aspect = (double)image_container_list[position]->aspect_raito[1];

    /*
       printf("widow size w: %d h: %d\n", window_width, window_height);
       printf("src w: %d h: %d\n", width, height);
       printf("%f : %f\n", w_aspect, h_aspect);
       */

    if(height > window_height)
    {
        int diff = height - window_height;
        height = height - diff;
        int result = (int)ceil((double)height * (w_aspect / h_aspect));
        // printf("result: %d, %d * %f\n", result, height, (w_aspect / h_aspect));
        width = result;
    }

    image_container_list[position]->dst = gdk_pixbuf_scale_simple(image_container_list[position]->src, width, height, GDK_INTERP_BILINEAR);
}

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gint width = 0;
    gint height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &width, &height);

    if(width != window.width || height != window.height)
    {
        update_image_size(current_page);

        gtk_image_clear((GtkImage*)image);

        gtk_image_set_from_pixbuf((GtkImage*)image, image_container_list[current_page]->dst);

        g_object_unref(image_container_list[current_page]->dst);

        return TRUE;
    }


    return FALSE;
}

void set_image(int position)
{
    image = gtk_image_new_from_pixbuf(image_container_list[position]->dst);
    // g_object_unref(image_container_list[position]->dst);
}

int init_image_object()
{
    // set image file path
    set_image_path_list();
    if(detail->image_count > 0)
    {
        image_container_list = (Image_Container_t**)calloc(detail->image_count, sizeof(Image_Container_t*));

        set_image_container(0);

        set_image(0);

        return 1;
    }

    return 0;
}
