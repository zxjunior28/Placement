#ifndef SRC_PLACEMENT_LEGALIZATION_ABACUS_HPP_
#define SRC_PLACEMENT_LEGALIZATION_ABACUS_HPP_

#include <placement/system.hpp>

namespace placement {

/*Legalization with Minimal Displacement*/
class LegalizationAbacus {
 public:
    using system_ptr_type =  std::shared_ptr<backend::System>;
    using cell_ptr = std::shared_ptr<backend::Cell>;
    using terminal_ptr = std::shared_ptr<backend::Terminal>;
    explicit LegalizationAbacus(std::shared_ptr<backend::System> system_ptr)
    : system_ptr_(system_ptr) {}
    ~LegalizationAbacus() = default;

    void initialize();
    system_ptr_type placement();
 private:
    system_ptr_type system_ptr_{nullptr};

    int binarySearchRow(const cell_ptr& cell);
    bool attempPlace(backend::Row& row, const cell_ptr& cell, int& best_cost, backend::Subrow* &best_subrow_place);
};

void LegalizationAbacus::initialize() {
    auto& terminal_list = system_ptr_->terminal_list;
    auto& cell_list = system_ptr_->cell_list;
    auto& row_list = system_ptr_->row_list;
    /*sort terminals by x coordinate*/
    std::sort(terminal_list.begin(), terminal_list.end(),
    [](terminal_ptr t1, terminal_ptr t2) {return t1->x < t2->x;});

    /*initialize terminal in row*/
    // std::cout <<  system_ptr_<< std::endl;
    for (auto terminal : terminal_list) {
        for (auto &row : row_list) {
            row.block(*terminal);
        }
    }
}


LegalizationAbacus::system_ptr_type LegalizationAbacus::placement() {
    auto row_list = system_ptr_->row_list;
    // change cell_list order
    auto& left_cell_list = system_ptr_->left_cell_list;
    std::sort(left_cell_list.begin(), left_cell_list.end(),
    [](const cell_ptr& c1, const cell_ptr& c2){
        if (c1->x == c2->x)
            return c1->width < c2->width;
        return c1->x < c2->x;
    });


    for (auto& cell : left_cell_list) {
        int best_cost = std::numeric_limits<int>::max();
        backend::Subrow *best_place = nullptr;
        int start_row = binarySearchRow(cell);

        // std::cout << start_row << std::endl;
        int range = 18;
        for (int i = start_row - range; i < start_row + range; ++i) {
            if (i >= 0 && i < row_list.size()) {
                attempPlace(row_list[i], cell, best_cost, best_place);
            }
        }

        for (int i = start_row - range - 1; i >= 0; --i)
            if (!attempPlace(row_list[i], cell, best_cost, best_place))
                break;

        for (int i = start_row - range + 1; i < row_list.size(); ++i)
            if (!attempPlace(row_list[i], cell, best_cost, best_place))
                break;

        if (best_place) {
            best_place->place(cell);
            best_place->backup();
            best_place->cost = best_place->getPosition();
            best_place->remain_space -= cell->width;
        }
    }


    // right
    auto& row_list2 = system_ptr_->row_list;
    // change cell_list order
    auto& right_cell_list = system_ptr_->right_cell_list;
    std::sort(right_cell_list.begin(), right_cell_list.end(),
    [](const cell_ptr& c1, const cell_ptr& c2){
        if (c1->x == c2->x)
            return c1->width < c2->width;
        return c1->x < c2->x;
    });


    for (auto& cell : right_cell_list) {
        int best_cost = std::numeric_limits<int>::max();
        backend::Subrow *best_place = nullptr;
        int start_row = binarySearchRow(cell);

        // std::cout << start_row << std::endl;
        int range = 18;
        for (int i = start_row - range; i < start_row + range; ++i) {
            if (i >= 0 && i < row_list2.size()) {
                attempPlace(row_list2[i], cell, best_cost, best_place);
            }
        }

        for (int i = start_row - range - 1; i >= 0; --i)
            if (!attempPlace(row_list2[i], cell, best_cost, best_place))
                break;

        for (int i = start_row - range + 1; i < row_list2.size(); ++i)
            if (!attempPlace(row_list2[i], cell, best_cost, best_place))
                break;

        if (best_place) {
            best_place->place(cell);
            best_place->backup();
            best_place->cost = best_place->getPosition();
            best_place->remain_space -= cell->width;
        }
    }

    // int cost = 0;
    // for (auto& row : row_list)
    //     cost += row.calCost();

    // system_ptr_->legalization_cost = cost;
 
    return std::move(system_ptr_);
}

bool LegalizationAbacus::attempPlace(backend::Row& row,
const cell_ptr& cell, int& best_cost, backend::Subrow* &best_subrow_place) {
    auto place = row.placeRow(cell);
    auto& subrow_place = place.first;
    auto& cost = place.second;
    if (subrow_place && cost < best_cost) {
        best_cost = cost;
        best_subrow_place = subrow_place;
        return true;
    } else if (subrow_place) {
        return false;
    }

    return true;
}


int LegalizationAbacus::binarySearchRow(const cell_ptr& cell) {
    const auto& row_list = system_ptr_->row_list;
    int left = 0;
    int right = row_list.size() - 1;
    while (left < right) {
        int mid = (left + right) / 2;
        if (row_list[mid].y == cell->y)
            return mid;
        else if (row_list[mid].y > cell->y)
            right = mid - 1;
        else
            left = mid + 1;
    }
    return std::max(0, left);
}


}  // namespace placement

#endif  // SRC_PLACEMENT_LEGALIZATION_ABACUS_HPP_
