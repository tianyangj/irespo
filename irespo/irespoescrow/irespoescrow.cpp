#include "irespoescrow.hpp"

namespace irespo {

	void irespoescrow::setapp(name application)
	{
		require_auth(_self);
		require_auth(application);

		eosio_assert(!configs(_self, _self).exists(), "Configuration already exists");
		configs(_self, _self).set(config{ application }, application);
	}

	void irespoescrow::withdraw(uint64_t from_user_id,
		name to_account,
		asset  quantity,
		string       memo)
	{
		
		eosio_assert(configs(_self, _self).exists(), "Application account not configured");

		require_auth(_self);
		require_auth(configs(_self, _self).get().application);

		auto iter = escrows.find(from_user_id);
		eosio_assert(iter != escrows.end(), "No tokens to withraw");

		eosio_assert(quantity.is_valid(), "invalid quantity");
		eosio_assert(quantity.amount > 0, "must withraw positive quantity");
		eosio_assert(iter->quantity.amount >= quantity.amount, "withraw no more than owned by account");
		eosio_assert(quantity.symbol == iter->quantity.symbol, "symbol precision mismatch");
		eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

		escrows.modify(iter, configs(_self, _self).get().application, [&](auto& row) {
			row.quantity -= quantity;
		});

		action(permission_level{ _self, N(active) }, N(irespotokens), N(transfer),
			make_tuple(_self, to_account, quantity, string("irespoescrow to user account"))).send();
	}

	void irespoescrow::apply(const account_name contract, const account_name act) {

		if (act == N(transfer)) {
			transferReceived(unpack_action_data<currency::transfer>(), contract);
			return;
		}

		auto &thiscontract = *this;
		switch (act) {
			EOSIO_ABI(irespoescrow, (setapp)(withdraw))
		};
	}

private:	

	void irespoescrow::transferReceived(const currency::transfer &transfer, const account_name code) {
		eosio_assert(static_cast<uint32_t>(code == N(eosio.token)), "needs to come from eosio.token");
		eosio_assert(static_cast<uint32_t>(transfer.memo.length() > 0), "needs a memo with the name");
		eosio_assert(static_cast<uint32_t>(transfer.quantity.symbol == S(4, EOS)), "only EOS token allowed");
		eosio_assert(static_cast<uint32_t>(transfer.quantity.is_valid()), "invalid transfer");
		eosio_assert(static_cast<uint32_t>(transfer.quantity.amount > 0), "must bet positive quantity");

		//if (transfer.to != _self) {
		//	return;
		//}

		//auto idx = accounts.template get_index<N(bytwitter)>();
		//auto act = idx.find(account::key(transfer.memo));

		//if (act != idx.end()) { // exists
		//	idx.modify(act, 0, [&](account &act) {
		//		act.balance = act.balance + transfer.quantity.amount;
		//	});

		//}
		//else { // no exist
		//	accounts.emplace(_self, [&](account &act) {
		//		act.pkey = accounts.available_primary_key();
		//		act.twitter_key = account::key(transfer.memo);
		//		act.twitter = transfer.memo;
		//		act.balance = static_cast<uint64_t>(transfer.quantity.amount);
		//	});
		//}
	}

} /// namespace irespo

using namespace irespo;
using namespace eosio;

extern "C" {
	void apply(uint64_t receiver, uint64_t code, uint64_t action) {
		auto self = receiver;
		irespoescrow contract(self);
		contract.apply(code, action);
		eosio_exit(0);
	}
}

//EOSIO_ABI(irespoescrow, (setapp)(withdraw))