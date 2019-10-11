#include "mainwindow.h"

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
                    if(buffer[0] == png_sig[0])
                    {
                        for(int j = 0; j < png_sig_size; ++j)
                        {
                            if(buffer[j] == png_sig[j])
                            {
                                sig_count++;
                            }
                        }
                    }
                    else if(buffer[0] == jpg_sig[0])
                    {

                        for(int j = 0; j < jpg_sig_size; ++j)
                        {
                            if(buffer[j] == jpg_sig[j])
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

            if(detail->image_count % 2)
            {
                detail->isOdd = TRUE;
            }
            else
            {
                detail->isOdd = FALSE;
            }
        }
        else
        {
            printf("Failed malloc to detail->image_list\n");
            free(detail->image_path_list);
        }
    }

}

void next_image(int isForward)
{
    if(pages->isSingle)
    {
        set_image_container(pages->current_page);

        gtk_image_clear((GtkImage*)pages->left);

        gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);    
    }
    else
    {
        if(isForward)
        {
            set_image_container(pages->current_page);
            if(pages->current_page - 1 < detail->image_count)
            {
                set_image_container(pages->current_page - 1);

            }

            gtk_image_clear((GtkImage*)pages->left);
            gtk_image_clear((GtkImage*)pages->right);

            /*
               if(pages->right != NULL)
               {
               gtk_image_clear((GtkImage*)pages->right);
               }
               */

            if(detail->isOdd && (pages->current_page + 1 == detail->image_count))
            {
                resize_when_single(pages->current_page);

                if(pages->page_direction_right)
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
                }
                else
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                }
            }
            else
            {
                int isOverHeight = resize_when_spread(pages->current_page);

                if(pages->page_direction_right)
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page - 1]->dst);
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                }
                else
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page - 1]->dst);
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
                }

                set_margin_left_page(pages->current_page, isOverHeight);
            }

        }
        else
        {
            gtk_image_clear((GtkImage*)pages->left);
            gtk_image_clear((GtkImage*)pages->right);

            if(detail->isOdd)
            {

                set_image_container(pages->current_page);

                int isOverHeight = resize_when_spread(pages->current_page);

                if(pages->page_direction_right)
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
                }
                else
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                }

                set_margin_left_page(pages->current_page, isOverHeight);

            }
            else
            {
                set_image_container(pages->current_page);
                set_image_container(pages->current_page - 1);

                int isOverHeight = resize_when_spread(pages->current_page);

                if(pages->page_direction_right)
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page - 1]->dst);
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                }
                else
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page - 1]->dst);
                }

                
                set_margin_left_page(pages->current_page, isOverHeight);
            }
        }


    }

}

gboolean my_key_press_function(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    switch(event->keyval) {
        case GDK_KEY_F:
            if(window.isFullScreen)
            {
                gtk_window_unfullscreen(GTK_WINDOW(window.window));
                gtk_widget_show(menubar);
                window.isFullScreen = FALSE;
            }
            else
            {
                gtk_window_fullscreen(GTK_WINDOW(window.window));
                gtk_widget_hide(menubar);
                window.isFullScreen = TRUE;
            }

            break;

        case GDK_KEY_Home:
            pages->current_page = 1;

            if(pages->isSingle)
            {
                pages->current_page = 0;
            }

            if(pages->page_direction_right)
            {
                next_image(FALSE);
            }
            else
            {
                next_image(TRUE);
            }

            break;

        case GDK_KEY_Right:
            if(pages->isSingle)
            {
                pages->current_page++;
            }
            else if(pages->page_direction_right)
            {
                pages->current_page -= 2;
            }
            else
            {
                pages->current_page += 2;
            }

            if(pages->current_page >= detail->image_count)
            {
                if(pages->isSingle)
                {
                    pages->current_page = 0;
                }
                else
                {
                    pages->current_page = 1;
                }
            }
            
            if(pages->current_page < 0)
            {
                pages->current_page = detail->image_count - 1;
            }

            printf("%s\n", detail->image_path_list[pages->current_page]);

            if(pages->page_direction_right)
            {
                next_image(FALSE);
            }
            else
            {
                next_image(TRUE);
            }

            printf("pressed right arrow key\n");

            return TRUE;

        case GDK_KEY_Left:
            if(pages->isSingle)
            {
                pages->current_page--;
            }
            else if(pages->page_direction_right)
            {
                pages->current_page += 2;
            }
            else
            {
                pages->current_page -= 2;
            }

            if(pages->current_page > detail->image_count)
            {
                pages->current_page = 1;
            }

            if(pages->current_page < 0)
            {
                pages->current_page = detail->image_count - 1;
            }

            printf("%d\n", pages->current_page);

            if(pages->page_direction_right)
            {
                next_image(TRUE);
            }
            else
            {
                next_image(FALSE);
            }

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

    free(pages);

    printf("Closed\n");
}


void set_image_container(int position)
{
    if(image_container_list[position] == NULL)
    {
        image_container_list[position] = malloc(sizeof(Image_Container_t));
        memset(image_container_list[position], 0, sizeof(Image_Container_t));

        image_container_list[position]->err = NULL;
        image_container_list[position]->src = gdk_pixbuf_new_from_file(detail->image_path_list[position], &image_container_list[position]->err);
        image_container_list[position]->src_height = gdk_pixbuf_get_height(image_container_list[position]->src);
        image_container_list[position]->src_width = gdk_pixbuf_get_width(image_container_list[position]->src);

        int width = image_container_list[position]->src_width;
        int height = image_container_list[position]->src_height;

        image_container_list[position]->aspect_raito = calc_aspect_raito(width, height, mygcd(width, height));

    }

    if(pages->isSingle)
    {
        resize_when_single(position);
    }
}

void resize_when_single(int position)
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

    /*
    if(width > window_width)
    {
        int diff = width - window_width;
        width = width - diff;
        height = (int)ceil((double)width * (h_aspect / w_aspect));
    }
    */


    image_container_list[position]->dst = gdk_pixbuf_scale_simple(image_container_list[position]->src, width, height, GDK_INTERP_BILINEAR);
    image_container_list[position]->dst_width = width;
    image_container_list[position]->dst_height = height;
}

gboolean detect_resize_window(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gint width = 0;
    gint height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &width, &height);

    if(width != window.width || height != window.height)
    {
        if(pages->isSingle)
        {
            resize_when_single(pages->current_page);

            gtk_image_clear((GtkImage*)pages->left);

            gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);

            unref_g_object((GtkWidget*)image_container_list[pages->current_page]->dst);
        }
        else
        {
            if(detail->isOdd && detail->image_count == pages->current_page - 1)
            {
                resize_when_single(pages->current_page);

                gtk_image_clear((GtkImage*)pages->left);
                gtk_image_clear((GtkImage*)pages->right);

                gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                unref_g_object((GtkWidget*)image_container_list[pages->current_page]->dst);
            }
            else
            {
                /*
                resize_when_single(pages->current_page - 1);
                resize_when_single(pages->current_page);
                */

                int isOverHeight = resize_when_spread(pages->current_page);

                gtk_image_clear((GtkImage*)pages->left);
                gtk_image_clear((GtkImage*)pages->right);

                if(pages->page_direction_right)
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page]->dst);
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page - 1]->dst);
                }
                else
                {
                    gtk_image_set_from_pixbuf((GtkImage*)pages->left, image_container_list[pages->current_page - 1]->dst);
                    gtk_image_set_from_pixbuf((GtkImage*)pages->right, image_container_list[pages->current_page]->dst);
                }

                set_margin_left_page(pages->current_page, isOverHeight);

                unref_g_object((GtkWidget*)image_container_list[pages->current_page]->dst);
                unref_g_object((GtkWidget*)image_container_list[pages->current_page - 1]->dst);

            }
        }

        return TRUE;
    }


    return FALSE;
}

void set_image(GtkWidget **img, int position)
{
    *img = gtk_image_new_from_pixbuf(image_container_list[position]->dst);
}

void scale_when_oversize(int *x, int *y, int window_width, int window_height, double w_aspect, double h_aspect, int isOverWidth)
{
    if(isOverWidth)
    {
        int diff = *x - window_width;
        int width = *x - diff;
        int height = (int)ceil((double)width * (h_aspect / w_aspect));

        *x = width;
        *y = height;
    }
    else
    {
        int diff = *y - window_height;
        int height = *y - diff;
        int width = (int)ceil((double)height * (w_aspect / h_aspect));

        *x = width;
        *y = height;
    }


}

int resize_when_spread(int page)
{
    int left_src_width = image_container_list[page - 1]->src_width;
    int left_src_height = image_container_list[page - 1]->src_height;
    double left_x_aspect = (double)image_container_list[page - 1]->aspect_raito[0];
    double left_y_aspect = (double)image_container_list[page - 1]->aspect_raito[1];

    int right_src_width = image_container_list[page]->src_width;
    int right_src_height = image_container_list[page]->src_height;
    double right_x_aspect = (double)image_container_list[page]->aspect_raito[0];
    double right_y_aspect = (double)image_container_list[page]->aspect_raito[1];

    gint window_width = 0;
    gint window_height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
    window.width = window_width;
    window.height = window_height;

    int half_width = window_width / 2;
    /*
    if(draw_area.width > 0)
    {
        half_width = draw_area.width / 2;
        window_width = draw_area.width;
        window_height = draw_area.height;
    }
    */

    int left_width = half_width;
    int left_height = 0;
    left_height = (int)ceil((double)left_width * (left_y_aspect / left_x_aspect));
    int isOverHeight = FALSE;
    if(left_height > window_height)
    {
        scale_when_oversize(&left_width, &left_height, window_width, window_height, left_x_aspect, left_y_aspect, FALSE);
        isOverHeight = TRUE;
    }

    image_container_list[page - 1]->dst_width = left_width;
    image_container_list[page - 1]->dst_height = left_height;

    printf("Left Width: %d, Left Height: %d\n", left_width, left_height);

    int right_width = half_width;
    int right_height = 0;
    right_height = (int)ceil((double)right_width * (right_y_aspect / right_x_aspect));

    if(right_height > window_height)
    {
        scale_when_oversize(&right_width, &right_height, window_width, window_height, right_x_aspect, right_y_aspect, FALSE);
    }

    image_container_list[page]->dst_width = right_width;
    image_container_list[page]->dst_height = right_height;

    image_container_list[page - 1]->dst = gdk_pixbuf_scale_simple(image_container_list[page - 1]->src, left_width, left_height, GDK_INTERP_BILINEAR);
    image_container_list[page]->dst = gdk_pixbuf_scale_simple(image_container_list[page]->src, right_width, right_height, GDK_INTERP_BILINEAR);

    return isOverHeight;
}

void set_margin_left_page(int position, int isOverHeight)
{
    /*
    gint window_width = 0;
    gint window_height = 0;
    gtk_window_get_size((GtkWindow*)window.window, &window_width, &window_height);
    window.width = window_width;
    window.height = window_height;
    */

    if(isOverHeight)
    {
        int mix_width = (image_container_list[position - 1]->dst_width + image_container_list[position]->dst_width);
        int margin_left = (fmax(mix_width, window.width) - fmin(mix_width, window.width)) / 2;

        printf("margin left: %d\n", margin_left);

        // g_object_set(pages->left, "margin_left", margin_left, NULL);
        gtk_widget_set_margin_start(pages->left, (gint)margin_left);
    }
    else
    {
        // g_object_set(pages->left, "margin_left", 0, NULL);
        gtk_widget_set_margin_start(pages->left, 0);
    }

}

int init_image_object(int startpage)
{
    pages->current_page = 0;
    // set image file path
    set_image_path_list();
    if(detail->image_count > 0)
    {
        image_container_list = (Image_Container_t**)calloc(detail->image_count, sizeof(Image_Container_t*));

        if(pages->isSingle)
        {
            set_image_container(0);

            set_image(&pages->left, 0);
        }
        else
        {
            printf("%s\n%s\n", detail->image_path_list[0], detail->image_path_list[1]);
            set_image_container(0);
            set_image_container(1);

            /*
               int result_width = image_container_list[0]->src_width + image_container_list[1]->src_width + 1;
               int result_height = image_container_list[0]->src_height + image_container_list[1]->src_height + 1;

               gdk_pixbuf_copy_area(image_container_list[0]->src, image_container_list[0]->src_width, image_container_list[0]->src_width + 1, image_container_list[0]->src_height + 1, image_container_list[0]->src_height, image_container_list[0]->src, result_width, result_height);

*/
            resize_when_spread(1);

            if(pages->page_direction_right)
            {
                set_image(&pages->left, 1);
                set_image(&pages->right, 0);
            }
            else
            {
                set_image(&pages->left, 0);
                set_image(&pages->right, 1);
            }


            pages->current_page = 1;
        }

        return TRUE;
    }

    return FALSE;
}

void unref_g_object(GtkWidget *object)
{
    if(object != NULL)
    {
        g_object_unref(object);
    }
}
