#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <unordered_map>
#include "xxh64.hpp"

namespace {
	struct TransparentHash {
		using is_transparent = void;

		size_t operator()(std::string_view sv) const noexcept {
			//return std::hash<std::string_view>{}(sv);
			return xxh64::hash(sv.data(), sv.size(), 0);
		}
	};

	struct TransparentEq {
		using is_transparent = void;

		bool operator()(std::string_view a, std::string_view b) const noexcept {
			return a == b;
		}
	};
};

struct CachedString {
	size_t index;

	CachedString(size_t i) : index(i) {}
};

class StringsCache {
	static const size_t BLOCK_SIZE = 64 * 1024;
	static const size_t DEFAULT_ALIGNMENT = alignof(std::max_align_t);

public:
	StringsCache() {
		blocks.emplace_back(std::make_unique<char[]>(BLOCK_SIZE));
		used = 0;

		intern("");
	}

	CachedString intern(std::string_view sv) {
		if (used + sv.size() > BLOCK_SIZE) {
			blocks.emplace_back(std::make_unique<char[]>(BLOCK_SIZE));
			used = 0;
		}

		auto it = map.find(sv);
		if (it != map.end()) {
			return CachedString(it->second);
		}

		auto* ptr = blocks.back().get() + used;
		std::memcpy(ptr, sv.data(), sv.size());

		size_t i = index.size();
		std::string_view stored(ptr, sv.size());

		map[stored] = i;
		index.push_back(stored);

		used += sv.size();
		align_used();

		return CachedString(i);
	}

	std::string_view resolve(CachedString id) const {
		if (id.index >= index.size())
			throw std::runtime_error("id.index >= index.size()");
		return index.at(id.index);
	}

	size_t size() const { return index.size(); }
	bool empty() const { return size() == 0; }
private:
	void align_used() {
		used = (used + DEFAULT_ALIGNMENT - 1) & ~(DEFAULT_ALIGNMENT - 1);
	}

	std::vector<std::string_view> index;
	std::unordered_map<std::string_view, size_t, TransparentHash, TransparentEq> map;

	size_t used;
	std::vector<std::unique_ptr<char[]>> blocks;
};