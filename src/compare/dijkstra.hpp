#ifndef MIX_DS_DIJKSTRA_HPP
#define MIX_DS_DIJKSTRA_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <any>

namespace mix::ds
{
    using dist_t  = std::int64_t;
    using id_t    = std::uint64_t;

    struct edge
    {
        dist_t cost;
        id_t   target;
    };

    using edges_t = std::vector<edge>;

    struct vertex
    {
        id_t     id;
        dist_t   distAprox;
        vertex*  prev;
        edges_t  forward;
        std::any handle;
        bool     isInQueue;
    };

    struct graph
    {
        std::vector<vertex> vertices;
    };

    struct path
    {
        id_t   from;
        id_t   to;
        dist_t cost;
    };
    
    struct road_graph_header
    {
        std::size_t vertexCount;
        std::size_t edgeCount;
    };
    
    struct vertex_ptr_compare
    {
        auto operator()
            (vertex* const lhs, vertex* const rhs)
        {
            return lhs->distAprox < rhs->distAprox;      
        }
    };

    inline auto to_words (std::string s)
    {
        auto const delims = {' '};
        auto const end    = std::cend(s);
        auto first        = std::cbegin(s);
        auto words        = std::vector<std::string>();

        while (first != end)
        {
            auto const last = std::find_first_of(first, end, std::cbegin(delims), std::cend(delims));

            if (first != last)
            {
                words.emplace_back(first, last);
            }

            if (last == end)
            {
                break;
            }

            first = std::next(last);
        }

        return words;
    }

    inline auto eat_comments
        (std::fstream& fstr)
    {
        auto line = std::string("c");
        while ('c' == line[0])
        {
            std::getline(fstr, line);
        }
        return line;
    }

    inline auto read_line
        (std::fstream& fstr)
    {
        auto line = std::string();
        std::getline(fstr, line);
        return line;
    }

    inline auto parse_header
        (std::string line)
    {
        auto const words = to_words(std::move(line));
        return road_graph_header {std::stoull(words[2]), std::stoull(words[3])};
    }

    inline auto parse_line
        (std::string line)
    {
        auto const words = to_words(std::move(line));
        return path {std::stoull(words[1]) - 1, std::stoull(words[2]) - 1, std::stoll(words[3])};
    }

    inline auto load_road_graph
        (std::string const& filePath) -> graph
    {
        auto fstr = std::fstream(filePath);

        if (!fstr.is_open())
        {
            throw "File not found.";
        }

        auto header   = parse_header(eat_comments(fstr));
        auto vertices = std::vector<vertex>(header.vertexCount);
        std::generate(begin(vertices), end(vertices), [i = 0ul]() mutable { return vertex {i++}; });

        auto line = parse_line(eat_comments(fstr));
        for (auto i = 0ul; i < header.vertexCount - 1; ++i)
        {
            vertices[line.from].forward.emplace_back(edge {line.cost, line.to});
            line = parse_line(read_line(fstr));
        }

        return graph {vertices};
    }

    template<template<class, class, class...> class PrioQueue, class... Options>
    auto find_point_to_all
        (graph& vs, id_t const from)
    {
        using queue_t  = PrioQueue<vertex*, vertex_ptr_compare>;
        using handle_t = typename queue_t::handle_t;

        auto queue = queue_t();

        for (auto& v : vs.vertices)
        {
            v.distAprox = std::numeric_limits<dist_t>::max();
            v.prev      = nullptr;
            v.isInQueue = false;
        }
        vs.vertices[from].distAprox = 0;
        vs.vertices[from].handle    = queue.insert(&vs.vertices[from]);

        while (!queue.empty())
        {
            auto const current = queue.find_min();
            queue.delete_min();

            for (auto const edge : current->forward)
            {
                auto const target = &vs.vertices[edge.target];
                if (current->distAprox + edge.cost < target->distAprox)
                {
                    target->distAprox = current->distAprox + edge.cost;
                    target->prev      = current;

                    if (target->isInQueue)
                    {
                        queue.decrease_key(std::any_cast<handle_t>(target->handle));
                    }
                    else
                    {
                        target->handle    = queue.insert(target);
                        target->isInQueue = true;
                    }
                }
            }
        }
    }
}

#endif