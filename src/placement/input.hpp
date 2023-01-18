#ifndef SRC_PLACEMENT_INPUT_HPP_
#define SRC_PLACEMENT_INPUT_HPP_


#include <placement/system.hpp>

namespace placement {

class Input {
 public:
    using system_ptr_type = std::unique_ptr<backend::System>;
    using string_type = std::string;

    /* --- Constructor & Destructor --- */
    explicit Input(std::ifstream& in) : in_(in) {
        if (in.fail())
            std::cerr << "no such file!! " <<  std::endl;
        else
            system_ptr_ = std::make_unique<backend::System>();
    }
    virtual ~Input() = default;
    /*----------------------------------*/

    /*read input file to get layout info*/
    system_ptr_type readFile(void);

 private:
    system_ptr_type system_ptr_{nullptr};
    std::ifstream& in_;
};

Input::system_ptr_type Input::readFile() {
    std::string key;

    while (in_ >> key) {
        if (key == "DieSize") {
            int chip_width, chip_height;
            in_ >> chip_width >> chip_height;
            system_ptr_->chip_width = chip_width;
            system_ptr_->chip_height = chip_height;
            // std::cout << key << " " << system_ptr_->chip_width << " " << system_ptr_->chip_height << std::endl;
        } else if (key == "DieRows") {
            int row_height, num_rows;
            in_ >> row_height >> num_rows;
            system_ptr_->row_height = row_height;
            system_ptr_->num_rows = num_rows;
            // std::cout << key << " " << system_ptr_->row_height << " " << system_ptr_->num_rows << std::endl;

            system_ptr_->row_list.reserve(num_rows);
            for (int i = 0; i < num_rows; ++i) {
                int x = 0;
                system_ptr_->row_list.push_back({x, row_height*i, system_ptr_->chip_width, row_height});
                // std::cout << system_ptr_->row_list[i].y << " " << system_ptr_->row_list[i].height
                // <<" " << system_ptr_->row_list[i].subrow_list.size()
                // <<" (" << system_ptr_->row_list[i].subrow_list[0].x1 << ",  "
                // << system_ptr_->row_list[i].subrow_list[0].x2 << ", " <<  system_ptr_->row_list[i].subrow_list[0].y<<
                // ")" << " remain_space is " << system_ptr_->row_list[i].subrow_list[0].remain_space << std::endl;
                // fgetc(stdin);
            }
            // fgetc(stdin);
        } else if (key == "Terminal") {
            int num_terminals;
            in_ >> num_terminals;
            system_ptr_->num_terminals = num_terminals;
            system_ptr_->terminal_list.resize(num_terminals);
            // std::cout << key << " " << system_ptr_->num_terminals << std::endl;
            for (int i = 0; i < num_terminals; ++i) {
                auto terminal_ptr = std::make_shared<backend::Terminal>();
                in_ >> terminal_ptr->name >> terminal_ptr->x >>
                terminal_ptr->y >> terminal_ptr->width >> terminal_ptr->height;
                system_ptr_->terminal_list[i] = terminal_ptr;

                // std::cout << system_ptr_->terminal_list[i]->name << " " << system_ptr_->terminal_list[i]->x <<
                // " " << system_ptr_->terminal_list[i]->y << " " << system_ptr_->terminal_list[i]->width <<" "
                // << system_ptr_->terminal_list[i]->height << std::endl;
            }
        } else if (key == "NumCell") {
            int num_cells;
            in_ >> num_cells;
            system_ptr_->num_cells = num_cells;
            system_ptr_->cell_list.resize(num_cells);
            system_ptr_->total_cell_area = 0;
            system_ptr_->max_cell_area = 0;

            // std::cout << key << " " << system_ptr_->num_cells << std::endl;
            for (int i = 0; i < num_cells; ++i) {
                auto cell_ptr = std::make_shared<backend::Cell>();
                in_ >> cell_ptr->name >> cell_ptr->x >>
                cell_ptr->y >> cell_ptr->width >> cell_ptr->height;
                system_ptr_->cell_list[i] = cell_ptr;
                cell_ptr->id = i;
                cell_ptr->area = cell_ptr->width*cell_ptr->height;
                system_ptr_->total_cell_area += cell_ptr->area;
                system_ptr_->max_cell_area = std::max(system_ptr_->max_cell_area, cell_ptr->area);

                // std::cout << cell_ptr->name << " " << cell_ptr->x <<
                // " " << cell_ptr->y << " " << cell_ptr->width <<" "
                // << cell_ptr->height <<" " << " id = " << cell_ptr->id 
                // <<" max_cell_area = "<< system_ptr_->max_cell_area 
                // << " total_cell_area = "<<system_ptr_->total_cell_area<< std::endl;
            }
        }
    }
    in_.close();
    return std::move(system_ptr_);
}


}  // namespace placement

#endif  // SRC_PLACEMENT_INPUT_HPP_
