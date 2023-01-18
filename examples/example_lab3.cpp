// Copyright (c) 2022 Katelyn Bai
#include <placement/Lab3.hpp>
#include <chrono>


int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: ./Lab3 <Input_flie> <Output_flie>" << std::endl;
    }
    std::ifstream in(argv[1], std::ifstream::in);
    std::fstream out(argv[2], std::fstream::out);

    // std::chrono::high_resolution_clock::time_point start, end;
    // start = std::chrono::high_resolution_clock::now();

    placement::Input input(in);
    auto data_ptr = input.readFile();
    // std::cout << "start lab3!!" <<" "<<data_ptr<< std::endl;

    // std::cout << "\n------Start FM Partition--------" << std::endl;
    placement::GraphPartition FM(std::move(data_ptr));
    FM.initialize();
    auto data_ptr2 = FM.FMpartition(10);
    // std::cout << "<Partition_cost> " << data_ptr2->partition_cost << std::endl;
    // std::cout << "------End FM Partition--------" << std::endl;
    // std::cout << "check => " << std::endl;
    // const auto& cell_list = data_ptr2->cell_list;

    // int left_area = 0, right_area = 0;
    // for (size_t i = 0; i < cell_list.size(); ++i) {
    //     if (cell_list[i]->id == 0) {
    //         left_area += data_ptr2->cell_list[i]->area;
    //     } else {
    //         right_area += data_ptr2->cell_list[i]->area;
    //     }
    // }
    // std::cout << "left area = " << left_area
    // << " right area = " << right_area <<" ratio = "
    // << static_cast<double>(right_area)/static_cast<double>(data_ptr2->total_cell_area)
    // <<" max cut cost  = " << data_ptr2->partition_cost << std::endl;

    // int left_area2 = 0, right_area2 = 0;
    // for (size_t i = 0; i < data_ptr2->left_cell_list.size(); ++i) {
    //     left_area2 += data_ptr2->left_cell_list[i]->area;
    // }

    // for (size_t i = 0; i < data_ptr2->right_cell_list.size(); ++i) {
    //     right_area2 += data_ptr2->right_cell_list[i]->area;
    // }

    // if (left_area2 != left_area || right_area2 != right_area) {
    //     std::cout << "error!!" << std::endl;
    //     fgetc(stdin);
    // }


    // std::cout << "\n\n ------Start Abacus Two Chips--------" << std::endl;
    placement::LegalizationAbacus Abacus(std::move(data_ptr2));
    Abacus.initialize();
    auto data_ptr3 = Abacus.placement();
    // std::cout << "------End Abacus Two Chips--------" << std::endl;

    /*Timer*/
    // end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> duration = end - start;

    /*Output File*/
    for (const auto& cell : data_ptr3->cell_list)
       out << cell->name << " " << cell->final_x << " " << cell->final_y << " " << cell->id << std::endl;

    // std::cout << "Time : "  << duration.count() << std::endl;
    return 0;
}
